# Basic-Inject
 
## Build
```
gcc .\src\basic_inject.c -o basic_inject.exe -s -m64
gcc .\src\test_dll.c -o test.dll -s -m64 -shared
```

## Usage
```
basic_inject.exe <process_name> <dll_full_path>
```