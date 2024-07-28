# Sample Games

<link href="https://c2a.chesstempo.com/pgnviewer/v2.5/pgnviewerext.vers1.css" media="all" rel="stylesheet" crossorigin>
<script defer language="javascript" src="https://c1a.chesstempo.com/pgnviewer/v2.5/pgnviewerext.bundle.vers1.js" crossorigin></script>

<link
href="https://c1a.chesstempo.com/fonts/MaterialIcons-Regular.woff2"
rel="stylesheet" crossorigin>

## Wild draw vs v9.10

Below is a game that I played as white vs network v9.10.  `dlchess` sacks a knight early
on and then makes it back up later by winning two pieces with a double pawn-fork.  I
missed what should have been an easy conversion in the end with two pawns, after
`dlchess` again gave up another knight for seemingly no reason on move 38.

<ct-pgn-viewer>[Event "Computer Chess Game"]
[Date "2024.07.28"]
[White "mcfarljm"]
[Black "dlchess"]
[Result "1/2-1/2"]
[TimeControl "600"]
[Annotator "1... +0.19"]
1. d4 h5 {+0.19/5 4} 2. e4 b5 {+0.28/4 5} 3. Bxb5 c5 {+0.39/4 5} 4. Nf3 Qb6
{+0.37/4 5} 5. Be2 cxd4 {+0.32/5 5} 6. Qxd4 Qxd4 {+0.53/6 5} 7. Nxd4 g6
{+0.51/5 6} 8. O-O Nf6 {+0.51/4 6} 9. Nc3 Nxe4 {+0.38/5 6} 10. Nxe4 d5
{+0.26/5 6} 11. Nc3 Bg7 {+0.23/5 6} 12. Be3 Na6 {+0.04/5 6} 13. a3 e5
{+0.20/5 6} 14. Nf3 d4 {+0.12/6 6} 15. Nxd4 exd4 {+0.68/7 6} 16. Bxd4 Bxd4
{+0.70/7 6} 17. Rad1 Bxc3 {+0.81/6 6} 18. bxc3 Nc5 {+0.79/5 7} 19. Rfe1 Be6
{+0.80/5 6} 20. Bf3 Rd8 {+0.66/5 6} 21. Rb1 Rc8 {+0.80/4 6} 22. Bd5 Na4
{+1.01/5 6} 23. Bxe6 fxe6 {+0.83/7 6} 24. Rxe6+ Kf7 {+0.72/7 6} 25. Ra6
Nxc3 {+0.64/6 6} 26. Rxa7+ Ke6 {+0.62/5 6} 27. Rb6+ Kf5 {+0.50/5 6} 28. h3
Ne2+ {+0.50/5 6} 29. Kh2 Rhg8 {+0.62/4 6} 30. Rf7+ Kg5 {+0.46/6 6} 31. a4
Rcf8 {+0.42/5 5} 32. Rxf8 Rxf8 {+0.31/6 5} 33. f3 Ra8 {+0.37/4 5} 34. Rb4
Nc3 {+0.37/5 5} 35. Rd4 Rxa4 {+0.50/5 5} 36. Rd3 Ne2 {+0.50/5 5} 37. Re3
Nf4 {+0.48/6 5} 38. g3 Ra7 {+0.47/7 5} 39. gxf4+ Kxf4 {+0.44/6 4} 40. Rb3
Rg7 {+0.51/4 4} 41. c4 Ke5 {+0.49/5 4} 42. c5 Kd5 {+0.58/5 4} 43. Rc3 Kd4
{+0.46/5 4} 44. Rc1 Rc7 {+0.35/5 4} 45. Kg3 Kd5 {+0.29/5 3} 46. Kf4 h4
{+0.17/5 3} 47. Kg5 Rxc5 {+0.05/5 3} 48. Rxc5+ Kxc5 {-0.07/7 3} 49. Kxg6
Kd6 {-0.03/6 2.9} 50. f4 Ke7 {-0.20/9 2.8} 51. f5 Kf8 {-0.35/11 2.8} 52. f6
Kg8 {-0.31/12 2.6} 53. Kg5 Kf7 {-0.11/9 2.5} 54. Kf5 Kf8 {-0.38/13 2.4} 55.
Kg4 Kf7 {-0.47/12 2.3} 56. Kxh4 Kxf6 {-0.13/8 2.1} 57. Kh5 Kf7
{-0.15/8 2.0} 58. Kh6 Kf6 {-0.20/8 1.8} 59. h4 Kf7 {-0.24/8 1.7} 60. Kh7
Kf8 {-0.24/9 1.6} 61. h5 Kf7 {-0.21/10 1.5} 62. h6 Kf8 {-0.13/10 1.3} 63.
Kg6 Kg8 {-0.08/8 1.3} 64. Kh5 Kh7 {-0.06/7 1.2} 65. Kg5 Kh8 {-0.06/7 1.1}
66. Kg6 Kg8 {-0.05/7 1.0} 67. Kf6 Kh7 {-0.03/5 0.9} 68. Kg5 Kh8
{-0.02/5 0.9} 69. Kg6 Kg8 {+0.00/1 0.7}
{Draw by repetition} 1/2-1/2
</ct-pgn-viewer>



## Loss to network v9.9

Below is a sample game vs network v9.9, where I lost with White.  This one is a little
embarassing, as `dlchess` sacked a Knight for a Pawn early in the game and still won.

<ct-pgn-viewer>
[Date "2024.07.28"]
[White "Human"]
[Black "dlchess"]
[Result "0-1"]
[GameDuration "00:07:18"]
[GameEndTime "2024-07-28T10:53:23.254 MDT"]
[GameStartTime "2024-07-28T10:46:04.791 MDT"]
[Opening "Queen's pawn"]
[PlyCount "64"]
[TimeControl "40/300"]
1. d4 a5 {+0.14/4 4.4s} 2. e4 {4.1s} g6 {+0.18/4 4.6s} 3. Nf3 {4.3s}
Bh6 {+0.23/4 4.8s} 4. Bxh6 {6.6s} Nxh6 {+0.17/5 5.0s} 5. c4 {1.7s}
Na6 {+0.21/4 5.2s} 6. Nc3 {2.7s} Ng4 {+0.20/4 5.4s} 7. Bd3 {8.8s}
Nxf2 {+0.30/5 5.6s} 8. Kxf2 {2.9s} Nb4 {+0.30/5 5.8s} 9. Be2 {18s}
h5 {+0.36/5 5.9s} 10. a3 {1.4s} Na6 {+0.42/5 6.0s} 11. Re1 {6.9s}
d6 {+0.50/4 6.1s} 12. Rc1 {9.6s} c6 {+0.39/4 6.2s} 13. e5 {20s}
dxe5 {+0.43/5 6.3s} 14. Nxe5 {5.3s} Bf5 {+0.56/4 6.4s} 15. Bd3 {9.1s}
Qxd4+ {+0.66/6 6.4s} 16. Kf1 {26s} Bxd3+ {+0.52/6 6.5s} 17. Qxd3 {3.8s}
Qxd3+ {+0.44/6 6.5s} 18. Nxd3 {1.8s} Rd8 {+0.42/5 6.6s} 19. Rcd1 {4.1s}
Rd6 {+0.53/4 6.4s} 20. b4 {3.5s} Rd4 {+0.43/6 6.4s} 21. b5 {12s}
Nb8 {+0.42/7 6.3s} 22. Nb2 {16s} Rf4+ {+0.56/5 6.3s} 23. Kg1 {2.7s}
cxb5 {+0.48/5 6.4s} 24. cxb5 {3.6s} Rg4 {+0.41/5 6.3s} 25. a4 {4.9s}
f6 {+0.37/5 6.1s} 26. Rd5 {13s} Kf7 {+0.48/5 6.0s} 27. Red1 {6.6s}
Rc8 {+0.71/5 5.9s} 28. Rd8 {4.1s} Rxc3 {+0.40/7 5.8s} 29. Rxb8 {7.3s}
Rc2 {+0.54/7 5.8s} 30. Nd3 {24s} Rgxg2+ {+1.01/10 5.7s} 31. Kf1 {7.4s}
Rxh2 {+1.07/9 5.6s} 32. Rxb7 {3.2s} Rh1# {+128.00/1 5.3s, Black mates} 0-1
</ct-pgn-viewer>

PGN viewer powered by Chess Tempo: <https://chesstempo.com/> 

