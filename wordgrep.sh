#!/bin/sh
bloomgrep . "$1" | xargs grep -H "$1"
