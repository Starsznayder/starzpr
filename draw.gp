set title "Rdar Data"
set xlabel "prêdkosc [m/s]"
set ylabel "odleglosc[m]"
set ylabel "odleglosc[m]"
set yr[30:*]
datafile = sprintf("radarData[%d].dat",n)
plot datafile every ::1 u (($1-82)*0.612245):(($2)*15):3 matrix w image
