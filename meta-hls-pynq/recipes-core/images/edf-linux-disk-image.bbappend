# Add the Jupyter Lab server (packages from meta-jupyter) and the autostart
# service to the EDF disk image.
IMAGE_INSTALL:append = " packagegroup-python3-jupyter jupyter-startup"

# Stage 2 (the pynq library) will append here, e.g.:
#   IMAGE_INSTALL:append = " python3-pynq python3-pynq-notebooks"
