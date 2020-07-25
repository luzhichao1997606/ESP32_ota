file(GLOB srcs_nvs "${CMAKE_CURRENT_LIST_DIR}/Storage/*.c") 
file(GLOB srcs_Ethernet "${CMAKE_CURRENT_LIST_DIR}/Ethernet/*.c") 
file(GLOB srcs_MQTT "${CMAKE_CURRENT_LIST_DIR}/MQTT/*.c") 
file(GLOB srcs_SPI "${CMAKE_CURRENT_LIST_DIR}/SPI/*.c")
file(GLOB srcs_OLED "${CMAKE_CURRENT_LIST_DIR}/OLED/*.c")
file(GLOB srcs_NRF "${CMAKE_CURRENT_LIST_DIR}/NRF24L01/*.c")
file(GLOB srcs_WIFI "${CMAKE_CURRENT_LIST_DIR}/WIFI_Smart/*.c")
file(GLOB srcs_OTA "${CMAKE_CURRENT_LIST_DIR}/HTTPS_OTA/*.c")

idf_component_register(SRCS "${srcs_nvs}" "${srcs_Ethernet}" "${srcs_MQTT}" "${srcs_SPI}" "${srcs_OLED}" "${srcs_NRF}" "${srcs_WIFI}" "${srcs_OTA}" "main.c" 
                       INCLUDE_DIRS "." "Storage" "Ethernet" "MQTT" "SPI" "OLED" "NRF24L01" "WIFI_Smart" "HTTPS_OTA"
                       EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem)
                       

