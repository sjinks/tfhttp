# Terraform HTTP Backend

[![CodeQL](https://github.com/sjinks/tfhttp/actions/workflows/codeql.yml/badge.svg)](https://github.com/sjinks/tfhttp/actions/workflows/codeql.yml)
[![SonarCloud](https://github.com/sjinks/tfhttp/actions/workflows/sonarcloud.yml/badge.svg)](https://github.com/sjinks/tfhttp/actions/workflows/sonarcloud.yml)
[![Static Code Analysis](https://github.com/sjinks/tfhttp/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/sjinks/tfhttp/actions/workflows/static-analysis.yml)

A state backend server that implements the [Terraform HTTP backend API](https://developer.hashicorp.com/terraform/language/settings/backends/http). This server does not handle authentication and should be used behind a reverse proxy (like nginx).

The server is rather an experiment in writing HTTP servers in C++. It is probably OK for hobby usage but not for mission-critical applications.

## Build

tl;dr: see [Dockerfile](Dockerfile).

1. Install required dependencies: clang++ or g++, cmake, make.
2. Install optional dependencies: [libev](http://dist.schmorp.de/libev/), [ada](https://github.com/ada-url/ada), [SQLite3](https://github.com/sqlite/sqlite), [llhttp](https://github.com/nodejs/llhttp), [sqlite3pp](https://github.com/iwongu/sqlite3pp), [json](https://github.com/nlohmann/json), [libretls](https://git.causal.agency/libretls).
3. If you don't install the optional dependencies, the system will try to download and build them. In this case, it may need `git`, `python3`, `python3-whichcraft` (for Ada), `tcl` (for SQLite3), and `openssl` (for LibreTLS). See [Dockerfile](Dockerfile) for details.
4. Build: `cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel && cmake --build build`.

## Usage

The following environment variables control the server:
* `TFHTTP_ADDRESS`: IP address the server will listen on. The default value is `0.0.0.0` (listen on all IP‌ addresses).
* `TFHTTP_PORT`: port the server will listen on. The default value is `3000`.
* `TFHTTP_DATABASE_NAME`: the name of the SQLite3 database where the server will store the state. By default, it is [`:memory:`](https://www.sqlite.org/inmemorydb.html), meaning its content will be lost on a restart.
* `TFHTTP_HTTPS`: set to `1` to enable HTTPS.
* `TFHTTP_CERTIFICATE`: path to the TLS certificate.
* `TFHTTP_PRIVATE_KEY`: path to the TLS certificate key.
* `TFHTTP_CA_CERTIFICATE`: path to the CA certificate.
* `TFHTTP_TRUSTED_CERTIFICATE`: path to the trusted certificate (used for OCSP stapling).
* `TFHTTP_TLS_PROTOCOLS`: a comma- or colon-delimited list of the TLS‌ protocols to use. The valid values are `tlsv1.0`, `tlsv1.1`, `tlsv1.2`, `tlsv1.3`, `all`, `default`, `legacy`, `secure`. See [`tls_config_parse_protocols(3)`](https://man.openbsd.org/tls_config_parse_protocols.3).
* `TFHTTP_TLS_CIPHERS`: list of the allowed ciphers. The valid values are `secure`, `default`, `compat`, `legacy`, `insecure`, `all`, or a libssl ciper string. See [`tls_config_set_ciphers(3)`](https://man.openbsd.org/tls_config_set_ciphers.3).
* `TFHTTP_TLS_CURVES`: a comma-separated list of the elliptic curves used during ECDHE key exchange. See [`tls_config_set_ecdhecurves(3)`](https://man.openbsd.org/tls_config_set_ecdhecurves.3).
* `TFHTTP_TLS_VERIFY_CLIENT`: set to `1` to enable client certificate verification, requiring the client to send a certificate.
* `TFHTTP_TLS_ENABLE_DHE`: set to `1` to enable DHE key exchange.

## Endpoints

The server provides the following endpoints:
* `GET /:project/state`: fetch Terraform state for the given `project`. The state will be created if it does not exist.
* `POST /:project/state`: save Terraform state for the given `project`.
* `DELETE /:project/state`: delete Terraform state for the given `project`.
* `LOCK /:project`: lock state.
* `UNLOCK /:project`: unlock state.

`:project` is an alphanumeric string matching the `[A-Za-z0-9_-]+` regular expression. This makes is possible to use this server for multiple Terraform projects.

## Sample Terraform Configuration

```hcl
terraform {
  backend "http" {
    address = "http://127.0.0.1:3000/myproject/state"
    lock_address = "http://127.0.0.1:3000/myproject"
    unlock_address = "http://127.0.0.1:3000/myproject"
  }
}
```

([More details](https://developer.hashicorp.com/terraform/language/settings/backends/http#configuration-variables)).

------


