<?php

$i = (isset($i)) ? $i += 1 : 0;
echo serialize($$i);