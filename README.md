# KLoad
A simple command line utility to quickly load and unload Windows drivers
## Usage
Basically, you just pass a path to the driver you want to load. For example: `KLoad.exe C:\Windows\System32\ARandomDriver.sys`<br>
In addition, if you are loading a filter driver you can pass an altitude value after the driver path. See [here](https://docs.microsoft.com/en-us/windows-hardware/drivers/ifs/allocated-altitudes) for more information. <br>
To unload pass "unload" as the driver name followed by the service key name. Example: `KLoad.exe unload Random.sys` This will target the registry `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Random.sys` and attempt to unload the driver at this key. Please note that not all (especially filter) drivers are designed to be unloaded.
