# Copyright (C) <2021> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

'''Script to roll owt-deps-webrtc revision.

This script is expected to be ran by GitHub action runners when a new change is
committed to owt-deps-webrtc. It updates the revision in DEPS to use the latest
owt-deps-webrtc. A change will be submitted owt-bot/owt-client-native's roll
branch. And a pull request will be opened if it doesn't exist.
'''

import os
import sys
import argparse
import re
import subprocess
import requests
import json

SRC_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DEPS_PATH = os.path.join(SRC_PATH, 'DEPS')
# Regex expression to match commit hash of owt-deps-webrtc.
REVISION_RE = re.compile(
    r"(?<=Var\('deps_webrtc_git'\) \+ '/owt-deps-webrtc' \+ '@' \+ ')[0-9a-f]{40}(?=',)")
TARGET_BRANCH = 'roll'


def webrtc_revision():
    '''Return current owt-deps-webrtc revision in DEPS.'''
    with open(DEPS_PATH, 'r') as f:
        deps = f.read()
        return re.search(REVISION_RE, deps).group(0)


def roll(revision):
    '''Update DEPS.'''
    with open(DEPS_PATH, 'r+') as f:
        deps = f.read()
        new_deps = re.sub(REVISION_RE, revision, deps)
        f.seek(0)
        f.truncate()
        f.write(new_deps)
    return


def commit_message(old_revision, new_revision):
    message = 'Roll WebRTC revision %s..%s.' % (
        old_revision[:8], new_revision[:8])
    return message


def commit(old_revision, new_revision):
    '''Create a git commit.'''
    message = commit_message(old_revision, new_revision)
    commands = [['git', 'config', 'user.name', 'owt-bot'], ['git', 'config', 'user.email',
                                                            '82484650+owt-bot@users.noreply.github.com']]
    # Create a git commit with message above.
    commands.extend([['git', 'add', 'DEPS'], ['git', 'commit', '-m', message]])
    # Force push because when a new version of owt-deps-webrtc is available, the old commit could be rewritten.
    commands.append(['git', 'push', '-f', 'https://github.com/owt-bot/owt-client-native.git',
                     'HEAD:refs/heads/%s' % TARGET_BRANCH])
    # Run commands.
    for c in commands:
        ret = subprocess.call(c, cwd=SRC_PATH)
        if ret != 0:
            return ret
    return 0


def pr(old_revision, new_revision, base_branch, token):
    '''Create a pull request. If a PR is already open for webrtc roller, do nothing.'''
    # Check if a pull request exists.
    # REST API ref: https://docs.github.com/en/rest/reference/pulls#list-pull-requests
    params = {'base': base_branch,
              'head': 'owt-bot:%s' % (TARGET_BRANCH), 'state': "open"}
    url = 'https://api.github.com/repos/open-webrtc-toolkit/owt-client-native/pulls'
    headers = {
        "Authorization": "token %s" % token,
        "Accept": "application/vnd.github.v3+json"
    }
    response = requests.get(url, params=params, headers=headers)
    if response.status_code != 200:
        print('Failed to get pull request list, REST response status code is %d.' % (
            response.status_code), file=sys.stderr)
        return -1
    fetched_prs = response.json()
    if len(fetched_prs) > 0:
        for pr in fetched_prs:
            if pr['user']['login'] == 'owt-bot':
                # Update existing PR.
                update_url = pr['url']
                print(update_url)
                update_params = {'title': commit_message(
                    old_revision, new_revision)}
                update_response = requests.patch(
                    update_url, json=update_params, headers=headers)
                print(json.dumps(update_params))
                if update_response.status_code != 200:
                    print(update_response.headers)
                    print(update_response.content)
                    print('Failed to update pull request, REST response status code is %d.' % (
                        update_response.status_code), file=sys.stderr)
                    return -1
                return 0
    # Create a new PR.
    create_params = {'title': commit_message(
        old_revision, new_revision), 'head': 'owt-bot:%s' % (TARGET_BRANCH), 'base': base_branch}
    create_response = requests.post(url, json=create_params, headers=headers)
    if create_response.status_code != 201:
        print('Failed to create pull request, REST response status code is %d.' % (
            create_response.status_code), file=sys.stderr)
        return -1
    return 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--revision', help='owt-deps-webrtc revision to roll to.')
    parser.add_argument(
        '--base_branch', help='Pull request will be merged to this branch.', default='main')
    parser.add_argument(
        '--token', help='Personal access token to create pull requests.')
    opts = parser.parse_args()
    old_revision = webrtc_revision()
    ret = roll(opts.revision) or commit(old_revision, opts.revision) or pr(
        old_revision, opts.revision, opts.base_branch, opts.token)
    return ret


if __name__ == '__main__':
    sys.exit(main())
