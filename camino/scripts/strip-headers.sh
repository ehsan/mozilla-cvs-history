#!/bin/bash

# Failures are delicious delicacies
set -e

if [[ "${BUILT_PRODUCTS_DIR}" == "" || "${FRAMEWORKS_FOLDER_PATH}" == "" ]]; then
  echo "BUILT_PRODUCTS_DIR and FRAMEWORKS_FOLDER_PATH must be set"
  exit 1
fi

echo Stripping framework headers
find "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}" -name *.h -exec rm {} \;
find "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}" -name Headers -type l -exec rm {} \;
find "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}" -name Headers -type d -exec rmdir {} \; -prune
find "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}" -name PrivateHeaders -type l -exec rm {} \;
find "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}" -name PrivateHeaders -type d -exec rmdir {} \; -prune
