#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <acc1> <acc2>"
    exit 1
fi

arg1=$1
arg2=$2

diff golden/acc${arg1}_100000_100_106_1.md5 golden/acc${arg2}_100000_100_106_1.md5
diff golden/acc${arg1}_100000_100_110_1.md5 golden/acc${arg2}_100000_100_110_1.md5
diff golden/acc${arg1}_100000_100_120_1.md5 golden/acc${arg2}_100000_100_120_1.md5
diff golden/acc${arg1}_100000_200_212_1.md5 golden/acc${arg2}_100000_200_212_1.md5
diff golden/acc${arg1}_100000_200_220_1.md5 golden/acc${arg2}_100000_200_220_1.md5
diff golden/acc${arg1}_100000_200_240_1.md5 golden/acc${arg2}_100000_200_240_1.md5
diff golden/acc${arg1}_100000_300_318_1.md5 golden/acc${arg2}_100000_300_318_1.md5
diff golden/acc${arg1}_100000_300_330_1.md5 golden/acc${arg2}_100000_300_330_1.md5
diff golden/acc${arg1}_100000_300_360_1.md5 golden/acc${arg2}_100000_300_360_1.md5

diff golden/acc${arg1}_10000_100_106_1.md5 golden/acc${arg2}_10000_100_106_1.md5
diff golden/acc${arg1}_10000_100_110_1.md5 golden/acc${arg2}_10000_100_110_1.md5
diff golden/acc${arg1}_10000_100_120_1.md5 golden/acc${arg2}_10000_100_120_1.md5
diff golden/acc${arg1}_10000_200_212_1.md5 golden/acc${arg2}_10000_200_212_1.md5
diff golden/acc${arg1}_10000_200_220_1.md5 golden/acc${arg2}_10000_200_220_1.md5
diff golden/acc${arg1}_10000_200_240_1.md5 golden/acc${arg2}_10000_200_240_1.md5
diff golden/acc${arg1}_10000_300_318_1.md5 golden/acc${arg2}_10000_300_318_1.md5
diff golden/acc${arg1}_10000_300_330_1.md5 golden/acc${arg2}_10000_300_330_1.md5
diff golden/acc${arg1}_10000_300_360_1.md5 golden/acc${arg2}_10000_300_360_1.md5

diff golden/acc${arg1}_1000_100_106_1.md5 golden/acc${arg2}_1000_100_106_1.md5
diff golden/acc${arg1}_1000_100_110_1.md5 golden/acc${arg2}_1000_100_110_1.md5
diff golden/acc${arg1}_1000_100_120_1.md5 golden/acc${arg2}_1000_100_120_1.md5
diff golden/acc${arg1}_1000_200_212_1.md5 golden/acc${arg2}_1000_200_212_1.md5
diff golden/acc${arg1}_1000_200_220_1.md5 golden/acc${arg2}_1000_200_220_1.md5
diff golden/acc${arg1}_1000_200_240_1.md5 golden/acc${arg2}_1000_200_240_1.md5
diff golden/acc${arg1}_1000_300_318_1.md5 golden/acc${arg2}_1000_300_318_1.md5
diff golden/acc${arg1}_1000_300_330_1.md5 golden/acc${arg2}_1000_300_330_1.md5
diff golden/acc${arg1}_1000_300_360_1.md5 golden/acc${arg2}_1000_300_360_1.md5

diff golden/acc${arg1}_5000_100_106_1.md5 golden/acc${arg2}_5000_100_106_1.md5
diff golden/acc${arg1}_5000_100_110_1.md5 golden/acc${arg2}_5000_100_110_1.md5
diff golden/acc${arg1}_5000_100_120_1.md5 golden/acc${arg2}_5000_100_120_1.md5
diff golden/acc${arg1}_5000_200_212_1.md5 golden/acc${arg2}_5000_200_212_1.md5
diff golden/acc${arg1}_5000_200_220_1.md5 golden/acc${arg2}_5000_200_220_1.md5
diff golden/acc${arg1}_5000_200_240_1.md5 golden/acc${arg2}_5000_200_240_1.md5
diff golden/acc${arg1}_5000_300_318_1.md5 golden/acc${arg2}_5000_300_318_1.md5
diff golden/acc${arg1}_5000_300_330_1.md5 golden/acc${arg2}_5000_300_330_1.md5
diff golden/acc${arg1}_5000_300_360_1.md5 golden/acc${arg2}_5000_300_360_1.md5
