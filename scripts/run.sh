#!/usr/bin/env bash
# Run the matrix program multiple times and collect results
current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/set_env.sh

export MODELS_DIR=${PROJECT_BIN_DIR}/models


run() {
  echo "${PROJECT_BIN_DIR}/${program} -m '${MODELS_DIR}/${model}' -v '${eye}' -l '${look}' ${options}"
  "${PROJECT_BIN_DIR}/${program}" -m "${MODELS_DIR}/${model}" -v "${eye}" -l "${look}" ${options}
}

model="cornell/CornellBox-Original.obj"
eye="0,1,2"
look="0,1,0"
program="ESCViewer2021"
# Run threaded
#options="--thread"
# Flatten triangles from scene
options="--flat"

run


