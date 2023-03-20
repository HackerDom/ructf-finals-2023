1. Install packer 1.7.0 or higher: https://learn.hashicorp.com/tutorials/packer/get-started-install-cli#
2. Run `packer init image.pkr.hcl`
3. Get the API Token for Digital Ocean: https://cloud.digitalocean.com/settings/applications
4. Run `packer build -var "api_token=<YOUR_API_TOKEN>" image.pkr.hcl`