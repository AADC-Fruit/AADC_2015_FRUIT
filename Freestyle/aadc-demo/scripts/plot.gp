set terminal x11 noraise
set xrange [0:50]
set yrange [-1.5:1.5]
set cbrange [0:1]
set palette defined (0 "red", 1 "green")
unset colorbox
plot "< tail -n 50 /tmp/gnuplotter.dat" u 1 w lines lc 3 lt 1 lw 1 t "Blink", \
"< tail -n 50 /tmp/gnuplotter.dat" u 2 w lines lc 4 lt 1 lw 1 t "EOG", \
"< tail -n 1 /tmp/gnuplotter.dat" u (25):(0):($1==0.1||$2==0.1?0:1) w points pt 7 ps 30 palette notitle
load "/tmp/gnuplotter.gpr"
