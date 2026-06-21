cmake_minimum_required(VERSION 4.1)
project(Proyecto_3_Datos_2)

set(CMAKE_CXX_STANDARD 17)

add_executable(Proyecto_3_Datos_2
        StoredDataManager/IndexBST.cpp
        StoredDataManager/IndexBST.h
        StoredDataManager/IndexBTree.cpp
        StoredDataManager/IndexBTree.h
        QueryProcessing/SystemCatalog.h
        QueryProcessing/SystemCatalog.cpp
        StoredDataManager/StorageManager.cpp
        StoredDataManager/StorageManager.h)
