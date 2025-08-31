rm -rf dse_reports
mkdir dse_reports
# For loop for different target periods
for i in 20 10 6.66 5 4 3.55 3.44 3.33 3 2 1.64
do
    echo "Running DSE for target period $i"
    sh build_HLS_project.sh --ns=$i --version=0
    cp -r BlockMatcher_HLS/solution1/syn/report/ dse_reports/report-t$i
done
