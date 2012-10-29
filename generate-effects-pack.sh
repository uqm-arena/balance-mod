#! /bin/sh

# Generates a zipped effects pack in content/addons by default.
# If there are parameters, the first will be interpreted as a fully
# qualified path for the base effects path and the second (if any)
# will be interpreted as a fully qualified path for the allow-retreat
# string pack.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.


if [ $1 ]
then
    cd content/addons
    zip -r $1 balance
else
    cd content/addons
    zip -r balance-effects.zip balance
fi

if [ $2 ]
then
    cd content/addons
    zip -r $2 balance-retreat
else
    cd content/addons
    zip -r balance-retreat.zip balance-retreat
fi
