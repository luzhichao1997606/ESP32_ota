#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
COMPONENT_ADD_INCLUDEDIRS := . Storage Ethernet MQTT SPI OLED NRF24L01 WIFI_Smart HTTPS_OTA
COMPONENT_SRCDIRS := . Storage Ethernet MQTT SPI OLED NRF24L01 WIFI_Smart HTTPS_OTA
COMPONENT_EMBED_TXTFILES :=  ${PROJECT_PATH}/server_certs/ca_cert.pem