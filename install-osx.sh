#!/bin/bash
exec rsync -var --update build/ /opt/btsdk/lib/ladspa/
