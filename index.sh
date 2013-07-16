echo "Starting Phase1 and 2 "
time ./phase1n2
echo "Starting Phasse3"
time ./phase3
echo "Starting Phase4a"
time ./phase4a
ulimit -n 1000
echo "Staring Phase4b"
time ./phase4b
rm -f arena/index*
