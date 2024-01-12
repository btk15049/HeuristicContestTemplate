set -eux

SH_DIR=$(cd $(dirname $0); pwd)

URL=https://img.atcoder.jp/ahc029/45e6da0b06.zip
TOOL_DIR=$SH_DIR/../official_tools

# $TOOL_DIR/tool.zip としてダウンロード
wget -O $TOOL_DIR/tool.zip $URL
unzip $TOOL_DIR/tool.zip -d $TOOL_DIR
mv $TOOL_DIR/tools/* $TOOL_DIR/
rm -r $TOOL_DIR/tools

cd $TOOL_DIR
cargo build --release 
