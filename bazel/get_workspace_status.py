#!/usr/bin/python

# This file was imported from https://github.com/bazelbuild/bazel at d6fec93.

# This script will be run bazel when building process starts to
# generate key-value information that represents the status of the
# workspace. The output should be like
#
# KEY1 VALUE1
# KEY2 VALUE2
#
# If the script exits with non-zero code, it's considered as a failure
# and the output will be discarded.

# For Envoy in particular, we want to force binaries to relink when the Git
# SHA changes (https://github.com/envoyproxy/envoy/issues/2551). This can be
# done by prefixing keys with "STABLE_". To avoid breaking compatibility with
# other status scripts, this one still echos the non-stable ("volatile") names.

# If this SOURCE_VERSION file exists then it must have been placed here by a
# distribution doing a non-git, source build.
# Distributions would be expected to echo the commit/tag as BUILD_SCM_REVISION

import subprocess
import os.path
import os
import re

def invoke(cmdargs):
    try:
        proc = subprocess.Popen(cmdargs, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
        stdout,stderr = proc.communicate()
    except subprocess.CalledProcessError:
        rc = returncode
    finally:
        rc = 0
    if stdout:
        stdout = re.sub("[\r\n]+$", "", stdout)
    if stderr:
        stderr = re.sub("[\r\n]+$", "", stderr)
    return rc,stdout,stderr

if os.getenv('SOURCE_VERSION') and os.path.exists(os.getenv('SOURCE_VERSION')):
    fd = open(os.getenv('SOURCE_VERSION'), "r")
    srcver = fd.read()
    print("BUILD_SCM_REVISION " + srcver)
    print("STABLE_BUILD_SCM_REVISION " + srcver)
    print("BUILD_SCM_STATUS Distribution")
    exit(0)

# The code below presents an implementation that works for git repository
rv,git_rev,errtxt = invoke(['git', 'rev-parse', 'HEAD'])
if rv != 0:
    exit(1)
print("BUILD_SCM_REVISION " + git_rev)
print("STABLE_BUILD_SCM_REVISION " + git_rev)

# Check whether there are any uncommitted changes
rv,outtxt,errtxt = invoke(['git', 'diff-index', '--quiet', 'HEAD', '--'])
if rv == 0:
    tree_status="Clean"
else:
    tree_status="Modified"
print("BUILD_SCM_STATUS " + tree_status)
print("STABLE_BUILD_SCM_STATUS " + tree_status)
