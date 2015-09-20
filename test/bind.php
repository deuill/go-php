<?php

if (isset($i)) {
	$i += 1;
} else {
	$i = 0;
}

echo serialize($$i);