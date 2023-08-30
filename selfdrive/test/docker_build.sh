#!/usr/bin/env bash
set -e

# To build sim and docs, you can run the following to mount the scons cache to the same place as in CI:
# mkdir -p .ci_cache/scons_cache
# sudo mount --bind /tmp/scons_cache/ .ci_cache/scons_cache

if [ -n "$PUSH_IMAGE" ] && [ -z "$CURRENT_ARCH_BUILD" ]; then
  echo "PUSH_IMAGE is only supported for single arch builds"
  exit 1
fi

SCRIPT_DIR=$(dirname "$0")
OPENPILOT_DIR=$SCRIPT_DIR/../../
if [ -n "$CURRENT_ARCH_BUILD" ]; then
  ARCH=$(uname -m)
  TAG_SUFFIX="-$ARCH"
fi

source $SCRIPT_DIR/docker_common.sh $1 "$TAG_SUFFIX"

DOCKER_BUILDKIT=1 docker buildx build --load --cache-to type=inline --cache-from type=registry,ref=$REMOTE_TAG -t $REMOTE_TAG -t $LOCAL_TAG -f $OPENPILOT_DIR/$DOCKER_FILE $OPENPILOT_DIR

if [ -n "$PUSH_IMAGE" ]; then
  docker push $REMOTE_TAG
  docker tag $REMOTE_TAG $REMOTE_SHA_TAG
  docker push $REMOTE_SHA_TAG
fi
