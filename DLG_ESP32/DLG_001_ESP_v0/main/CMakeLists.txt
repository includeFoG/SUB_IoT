# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS			   "main.cpp" "ConfigBoard_DLG001.cpp" "DLG001_def.cpp" "TCA6408A_lib.cpp" "Sensor.cpp"# list the source files of this component ARCHIVOS FUENTE MAIN
    INCLUDE_DIRS       "include"# optional, add here public include directories (si hay h en /MAIN/INCLUDE añadir "INCLUDE")
    PRIV_INCLUDE_DIRS   "." # optional, add here private include directories (solo para el main)
    REQUIRES            "driver" "esp_partition" # optional, list the public requirements (component names externos e internos)
    PRIV_REQUIRES       # optional, list the private requirements
)
