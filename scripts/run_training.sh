#!/bin/bash

# Suggested use is to launch as a script with nohup or to run with docker in
# detached mode (-d).

BUILD_DIR=/app/build
MAJOR_VERSION=11
RESULTS_DIR=results
# Flag for whether to retain experience data
KEEP_EXPERIENCE=1

initial_version=20
num_iterations=5
num_tasks=4
cooldown_seconds=1800

# reference_version=8.5
reference_major_version=9

timestamp=`date +%Y%m%dT%H%M`

mkdir -p $RESULTS_DIR

if [ "$initial_version" -eq 0 ]; then
    # Create initial random model:
    # python3 nn/conv_4x64.py -o $RESULTS_DIR/v$MAJOR_VERSION.0
    python3 nn/resid_net.py -o $RESULTS_DIR/v$MAJOR_VERSION.0
fi

# Main iteration loop:
for iter in $(seq $initial_version $(( num_iterations + $initial_version - 1 ))); do
    echo `date`
    echo "Starting iteration $iter"

    version=$MAJOR_VERSION.$iter
    output_dir=$RESULTS_DIR/output_$version
    if [ -d "$output_dir" ]; then
        echo "Output directory exists: $output_dir"
        exit 1
    fi
    mkdir -p $output_dir

    # Selfplay:
    for i in $(seq $num_tasks); do
        $BUILD_DIR/selfplay \
            $RESULTS_DIR/v$version.onnx \
            -g 800 \
            -m 256 \
            -e 50 \
            --cache-size 1600000 \
            -t 1 \
            --encoding-version 2 \
            -o $output_dir/experience \
            -l "${timestamp}_$i" \
            > "$output_dir/sim_${i}.out" &
    done
    wait

    new_version="$MAJOR_VERSION.$(( iter + 1 ))"

    # Training:
    python3 train/train.py \
            -e $output_dir/experience \
            -i $RESULTS_DIR/v$version.pt \
            -o $RESULTS_DIR/v$new_version.pt \
            --resid-net \
            -b 256 \
            --interval 4 \
            --lr 5e-3 \
            > "$output_dir/train.out"

    if [ $? -eq 0 ]; then
        if [ $KEEP_EXPERIENCE -eq 1 ]; then
            tar -czf $output_dir/experience.tar.gz $output_dir/experience
        fi
        if [ $? -eq 0 ]; then
            rm -rf $output_dir/experience
        fi
    fi

    # Cooldown
    sleep $cooldown_seconds

    # Evaluation:

    cutechess-cli -engine dir=. cmd=$BUILD_DIR/dlchess arg=$RESULTS_DIR/v$new_version.onnx arg=-t arg=1 arg="--rounds=800" name=v$new_version \
                  -engine dir=. cmd=$BUILD_DIR/dlchess arg=$RESULTS_DIR/v$version.onnx arg=-t arg=1 arg="--rounds=800" name=v$version \
                  -each proto=uci tc=inf \
                  -concurrency $num_tasks \
                  -rounds 100 -games 2 -maxmoves 150 \
                  -openings file=openings/openings-6ply-1000.pgn policy=round -repeat \
                  > $RESULTS_DIR/eval_$new_version.out

    # Compare against reference version:
    # reference_version="$reference_major_version.$(( iter + 1 ))"
    # cutechess-cli -engine dir=. cmd=$BUILD_DIR/dlchess arg=$RESULTS_DIR/v$new_version.onnx arg=-t arg=1 arg="--rounds=800" name=v$new_version \
    #               -engine dir=. cmd=$BUILD_DIR/dlchess arg=$RESULTS_DIR/v$reference_version.onnx arg=-t arg=1 arg="--rounds=800 "name=v$reference_version \
    #               -each proto=uci tc=inf \
    #               -concurrency $num_tasks \
    #               -rounds 100 -games 2 -maxmoves 150 \
    #               -openings file=openings/openings-6ply-1000.pgn policy=round -repeat \
    #               > $RESULTS_DIR/ref_eval_$new_version.out

done

echo `date`
echo "Finished"
