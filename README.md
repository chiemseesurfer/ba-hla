Source of my bachelor thesis
-----------
Copyright &copy; 2012-2013 Max Oberberger (max@oberbergers.de)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.

You should have received a copy of the GNU General Public License 
along with this program.  If not, see <http://www.gnu.org/licenses/>.


### Introduction 
This repository contains a example implementation of **[HLA][]** based on
**[OpenRTI][]**

[OpenRTI]: https://gitorious.org/openrti
[HLA]: http://en.wikipedia.org/wiki/High-level_architecture_(simulation)

### System Requirements
- openrti. Available at his [gitorious-repository][OpenRTI].
- cmake. Should be already installed, because you need it also to install
  openrti.

### Installation from Source
- clone it from GitHub: `git clone https://github.com/chiemseesurfer/ba-hla.git`
- Do `make all` and `make install` to perform installation under
  `/usr/local/bin`
- Just call `make all` if you don't want to perform installation under `/usr/local/bin`


### How to use it
To use the Federates, you need a running openrti first.

    $ rtinode

**Note:** Port 14321 has to be enabled in firewall. See `rtinode -h` and README
file of openrti for more options to openrti.

After rtinode was started, you can run the Testfederate with `ba-hla --name
<NAME> --address <IP-ADDRESS> --fom </PATH/TO/FOM>`.

If you have rtinode running on the same computer as ba-hla you can use something
like:
    
    ba-hla --name myFederate --address 127.0.0.1 --fom /tmp/testFom.xml
