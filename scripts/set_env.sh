#!/usr/bin/env bash
# Run the matrix program once
# E.g. simple_run.sh -s 1024 --transpose --ikj
if [ -z "$PROJECT_HOME" ]; then
  current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
  export PROJECT_HOME=$( dirname ${current_dir} )
fi

export PROJECT_DATA_DIR=${PROJECT_HOME}/data
export PROJECT_OUT_DIR=${PROJECT_HOME}/outdata
export PROJECT_TEST_DIR=${PROJECT_HOME}/testdata
export PROJECT_METRICS_DIR=${PROJECT_HOME}/reports
export PROJECT_BIN_DIR=${PROJECT_HOME}/bin
export PROJECT_SCRIPTS_DIR=${PROJECT_HOME}/scripts
export PROJECT_ADVISOR_DIR=${PROJECT_HOME}/advisor

