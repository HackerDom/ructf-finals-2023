# RuCTF vuln image

* [Add new service to vuln_image](#Add-new-service-to-vuln-image)
* [Add new service in CI](#Add-new-service-in-CI)
* [Build image with CI](#Build-image-with-CI)
* [Build image with packer](#Build-image-with-packer)

## Add new service to vuln image

1. [Add user for service](https://github.com/HackerDom/ructf-finals-2023/blob/d310ee35ae48326951e6f617efcbb8d2ea0385b8/vuln_image/image.pkr.hcl#L59)
1. [Add "file provisioner" for service sources](https://github.com/HackerDom/ructf-finals-2023/blob/d310ee35ae48326951e6f617efcbb8d2ea0385b8/vuln_image/image.pkr.hcl#L94)
1. [Enable onboot service in systemd](https://github.com/HackerDom/ructf-finals-2023/blob/d310ee35ae48326951e6f617efcbb8d2ea0385b8/vuln_image/image.pkr.hcl#L101)

## Add new service in CI

1. Commit service sources in master brunch
1. Run `generate_workflow.py` script
  ```shell
  cd .github/workflows && ./generate_workflows.py && cd -
  ```
1. Commit changes

## Build image with CI

Run `Rebuild image` [workflow](https://github.com/HackerDom/ructf-finals-2023/actions/workflows/rebuild_image.yml)

## Build image with packer

1. Install packer 1.7.0 or higher: https://learn.hashicorp.com/tutorials/packer/get-started-install-cli#
1. Run `packer init image.pkr.hcl`
1. Get the API Token for Digital Ocean: https://cloud.digitalocean.com/settings/applications
1. Run
```shell
export DO_API_TOKEN=<YOUR_API_TOKEN>
packer build -var "api_token=$DO_API_TOKEN" image.pkr.hcl
```