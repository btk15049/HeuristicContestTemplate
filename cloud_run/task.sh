set -eu

gsutil cp ${GCS_SOURCE_FILE_PATH} ${CONTAINER_SOURCE_FILE_PATH}
eval "$BUILD_COMMAND"

mkdir -p input
mkdir -p output
gsutil -m cp "${GCS_INPUT_DIR_PATH}/${CLOUD_RUN_TASK_INDEX}/*" input/

for f in $(ls input/)
do
    echo $f
    ./tester ./a.out < input/$f 2> output/$f
done

gsutil -m cp output/* ${GCS_OUTPUT_DIR_PATH}/
