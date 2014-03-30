# Saskdora [![Build Status](https://travis-ci.org/discoverygarden/saskdora.png?branch=7.x)](https://travis-ci.org/discoverygarden/saskdora)

## Introduction

This module includes customizations to Islandora for use by the University of
Saskatchewan.

Its focused largely on implementing a security workflow, wherein specific users
can manage specific objects.

Collections and objects will be owned and managed by a *Principal Investigator*
(which is often a professor). Each object will also have zero or more
*Collaborators* (typically graduate students) who are responsible for populating
the objects with content and creating new objects. Any new objects will belong
belong to the *Principal Investigator* regardless of who creates them.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [Islandora XACML Editor](http://github.com/Islandora/islandora_xacml_editor)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

You will also have to give the 'primary investigator' role the
'can manage collaborators' permission.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a
solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors
Current maintainers:

* [discoverygarden](https://github.com/discoverygarden)

This project has been sponsored by:

* [University of Saskatchewan](www.usask.ca)
The University of Saskatchewan is a Canadian public research university, founded
in 1907, and located on the east side of the South Saskatchewan River in
Saskatoon, Saskatchewan, Canada.

## Development

If you would like to contribute to this module, please check out our helpful
[Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers)
info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
