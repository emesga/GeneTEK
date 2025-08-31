# Initialize default values
target_ns=5
version=0

# Process arguments
while [ $# -gt 0 ]; do
    case "$1" in
        --ns=*)
            target_ns="${1#*=}"
            ;;
        --version=*)
            version="${1#*=}"
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
    shift
done
rm -rf BlockMatcher_HLS
rm -f IP-repo/IP-repo.zip
vitis_hls -f BlockMatcher_HLS.tcl -tclargs $version $target_ns

#cd IP-repo
#rm -rf IP-repo/*
#unzip IP-repo.zip -d IP-repo/
#cd ..
