#! /bin/sh

# Generates a zipped effects pack in content/addons by default.
# If there is a parameter, it will be interpreted as a fully qualified
# path for the new effects pack.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.


if [ $1 ]
then
    zip -r $1 content/addons/balance
else
    zip -r content/addons/balance-effects.zip content/addons/balance
fi

if [ $2 ]
then
    zip -r $2 content/addons/balance-retreat
else
    zip -r content/addons/balance-retreat.zip content/addons/balance-retreat
fi
