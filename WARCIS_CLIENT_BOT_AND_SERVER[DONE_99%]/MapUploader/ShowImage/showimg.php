<?php

function caching_headers ($file, $timestamp) {
    $gmt_mtime = gmdate('r', $timestamp);
    header('ETag: "'.md5($timestamp.$file).'"');
    header('Last-Modified: '.$gmt_mtime);
    header('Cache-Control: public');

    if(isset($_SERVER['HTTP_IF_MODIFIED_SINCE']) || isset($_SERVER['HTTP_IF_NONE_MATCH'])) {
        if ($_SERVER['HTTP_IF_MODIFIED_SINCE'] == $gmt_mtime || str_replace('"', '', stripslashes($_SERVER['HTTP_IF_NONE_MATCH'])) == md5($timestamp.$file)) {
            header('HTTP/1.1 304 Not Modified');
            exit();
        }
    }
}

if(isset($_GET['image_name'])){
  $xfilepath = basename(urldecode($_GET['image_name']));
  if(file_exists("../maplistfullnew/".$xfilepath))
  {
	caching_headers ("../maplistfullnew/".$xfilepath, filemtime("../maplistfullnew/".$xfilepath));
	
	header('Content-Type: image/png');
	readfile("../maplistfullnew/" .$xfilepath);
  }
}