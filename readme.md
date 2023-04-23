# Foray Irradiance Cache
An Irradance cache implemented by @Firestar99 based upon the foray framework.

## Setup

```
git clone --recursive https://github.com/Vulkemp/foray-examples
```
Also clones the submodule `foray` (required).

## Examples
### [GBuffer](./examples/00-direct-light-cache)
First simple experiment with an irradiance cache by using it for direct light. You would of course not use such a cache for direct light as it wouldn't look great, 
but it's a first step towards getting an indirect light cache working.

## Tested Build Environments
* Linux G++
* Linux Clang
* Windows MSVC
* Windows Clang
