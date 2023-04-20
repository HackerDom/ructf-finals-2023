#!/usr/bin/env python3

import os
ctf_root = os.path.join(os.path.dirname(__file__), '../../')
services_dir = os.path.join(ctf_root, 'services')

SERVICES = [
  'example',
  'bookster',
  'solaris',
  'sneakers',
 ]

TEMPLATE = '''
name: Check {service}
on:
  push:
    branches:
      - master
    paths:
      - 'services/{service}/**'
      - 'checkers/{service}/**'
      - 'sploits/{service}/**'
  workflow_dispatch:
    inputs:
      cleanup_before_deploy:
        description: 'Stops and fully removes service (with volumes!) before deploying again. Type "yes" to do it.'
        required: false
        default: "no"
jobs:
  check_service_{service}:
    name: Check service {service}
    runs-on: ubuntu-20.04
    steps:
    - name: Checkout infra
      uses: actions/checkout@v2
    - name: Run prebuilt hook
      run: if [ -f services/{service}/before_image_build.sh ]; then (cd ./services/{service} && ./before_image_build.sh); fi
    - name: Setup {service}
      run: (cd ./services/{service} && docker-compose pull && docker-compose build && docker-compose up --build -d)
  check_checker_{service}:
    name: Check checker {service}
    runs-on: ubuntu-20.04
    steps:
    - name: Checkout infra
      uses: actions/checkout@v2
    - name: Run prebuilt hook
      run: if [ -f services/{service}/before_image_build.sh ]; then (cd ./services/{service} && ./before_image_build.sh); fi
    - name: Setup {service}
      run: (cd ./services/{service} && docker-compose pull && docker-compose build && docker-compose up --build -d)
    - name: Prepare python for checker
      uses: actions/setup-python@v2
      with:
        python-version: 3.11
    - name: Setup checker libraries
      run: if [ -f checkers/{service}/requirements.txt ]; then python -m pip install -r checkers/{service}/requirements.txt; fi
    - name: Test checker on service
      run: (cd ./checkers/{service} && ./{service}.checker.py TEST 127.0.0.1)
  check_sploit_{service}:
    name: Check sploit {service}
    runs-on: ubuntu-20.04
    steps:
    - name: Checkout infra
      uses: actions/checkout@v2
    - name: Run prebuilt hook
      run: if [ -f services/{service}/before_image_build.sh ]; then (cd ./services/{service} && ./before_image_build.sh); fi
    - name: Setup {service}
      run: (cd ./services/{service} && docker-compose pull && docker-compose build && docker-compose up --build -d)
    - name: Prepare python for checker
      uses: actions/setup-python@v2
      with:
        python-version: 3.11
    - name: Setup checker libraries
      run: if [ -f checkers/{service}/requirements.txt ]; then python -m pip install -r checkers/{service}/requirements.txt; fi
    - name: Setup sploit libraries
      run: if [ -f sploits/{service}/requirements.txt ]; then python -m pip install -r sploits/{service}/requirements.txt; fi
    - name: Test sploit on service
      run: (./tools/check-sploit.py {service})
  update_{service}:
    name: Deploy service using ansible to first teams
    needs: [check_service_{service}, check_checker_{service}, check_sploit_{service}]
    runs-on: self-hosted
    if: ${{{{ false }}}} # disable on game just in case
    steps:
    #- name: install ansible
    #  run: sudo apt-get install -y ansible
    - name: Checkout infra
      uses: actions/checkout@v2
    - name: change permission for ssh key
      run: chmod 0600 ./teams/for_devs.ssh_key
    - name: Run prebuilt hook
      run: if [ -f services/{service}/before_image_build.sh ]; then (cd ./services/{service} && sudo ./before_image_build.sh); fi
    - name: update checkers
      run: cd ./ansible && ansible-playbook cs-checkers.yml
    - name: deploy {service}
      run: cd ./ansible && ansible-playbook --extra-vars cleanup_service=${{{{ github.event.inputs.cleanup_before_deploy }}}} -t {service_lower} -l {service_lower} deploy-services.yml
'''

for s in SERVICES:
    with open('check_{}.yml'.format(s), 'w') as f:
        f.write(TEMPLATE.format(service=s, service_lower=s.lower()))
