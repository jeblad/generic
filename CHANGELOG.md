# Changelog

All notable changes to this project will be documented in this file. See [commit-and-tag-version](https://github.com/absolute-version/commit-and-tag-version) for commit guidelines.

## [0.1.1](https://github.com/jeblad/generic/compare/v0.1.0...v0.1.1) (2026-07-04)


### Features

* **locale:** improve hint messages and add translation guidelines ([b93d164](https://github.com/jeblad/generic/commit/b93d164b98f36a8d9635c5960c26d085d6ee928d))

## [0.1.0](https://github.com/jeblad/generic/compare/v0.0.4...v0.1.0) (2026-07-04)

## [0.0.4](https://github.com/jeblad/generic/compare/v0.0.3...v0.0.4) (2026-07-04)


### Features

* **about,list:** use hera::AgentHeader for agent file metadata reading ([9165cc0](https://github.com/jeblad/generic/commit/9165cc098b306b97078a45003070a89fb3e557ae))
* **down:** add hera::down() implementation and tests ([b06474d](https://github.com/jeblad/generic/commit/b06474dd85692058600ec5da2169177fb9fb6699))
* **down:** implement hera_down with SIGTERM + wait + conditional SIGKILL ([39e7b4a](https://github.com/jeblad/generic/commit/39e7b4a9ab402a160c732f88f5820f97c17e7ecf))
* **import,export:** implement hera_import and hera_export in generic plugin ([d81d954](https://github.com/jeblad/generic/commit/d81d954d32f326c1b55ae942316d2df0141b66ea))
* **init,import,export:** implement hera_init, hera_import, hera_export in generic plugin ([9fcce25](https://github.com/jeblad/generic/commit/9fcce25711574e2063a7a45cb905de44a980ed05))
* **prune:** list dangling daemon state with per-file status ([5ac9f91](https://github.com/jeblad/generic/commit/5ac9f912a78a3f66fcea00f3f825920d0347cecc))
* **prune:** rewrite prune to support clean and destroy ([f7bd601](https://github.com/jeblad/generic/commit/f7bd601fe971578203cd848c867b05435a18b3a5))
* **signal:** map HERA_SIGHUP to SIGHUP for checkpoint ([dea1922](https://github.com/jeblad/generic/commit/dea1922a5abd4a60d2b63b5f3077d6e453b4232f))


### Bug Fixes

* **install,uninstall,clone:** migrate from raw BEVE to JSON-parts multipart format ([aa8b255](https://github.com/jeblad/generic/commit/aa8b2556f58520ec3134964feed6744f73c2f158))
* **list:** skip stale PID entries via liveness check ([79c41bd](https://github.com/jeblad/generic/commit/79c41bda02ef26dbf216ccbbdc2c7a5dab4dc8bb))
* **prune:** hint rebuild when MMIO state exists for dead daemon ([d6e67e8](https://github.com/jeblad/generic/commit/d6e67e89bb2d6f0c21af414041b864ef243f74d2))
* **prune:** use beve_path instead of removed filename field ([e0d551f](https://github.com/jeblad/generic/commit/e0d551f1e5ce03c5e85948f7dd0d819e6124b2d1))
* **signal:** remove redundant PID cleanup after kill ([0d803e2](https://github.com/jeblad/generic/commit/0d803e2bebe77e044688c875f20bf174dbafd07d))
* **tests:** update test_about to write multipart format instead of raw BEVE ([3c31246](https://github.com/jeblad/generic/commit/3c312462ee43921a91616a3df8e9286dae14509b))

## [0.0.3](https://github.com/jeblad/generic/compare/v0.0.2...v0.0.3) (2026-05-07)

## [0.0.2](https://github.com/jeblad/generic/compare/v0.0.1...v0.0.2) (2026-05-06)


### Features

* lots of code changes, and changed to Private Work / Artistic Property status ([62bc283](https://github.com/jeblad/generic/commit/62bc2832e72524d849b1410512de4788e3859471))

## [0.0.1](https://github.com/jeblad/generic/compare/v0.0.0...v0.0.1) (2026-05-05)


### Features

* stabilize and standardize metadata reporting functions ([59f5862](https://github.com/jeblad/generic/commit/59f58628c24912eac98dd4d3694b84312e872f9a))
