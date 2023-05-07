<?php

if(isset($_GET['image_name'])){
   $xfilepath = urldecode($_GET['image_name']);
  if(file_exists("../maplistfullnew/".$xfilepath))
  {
	  header('Content-Type: image/png');
	 
	  readfile("../maplistfullnew/" .$xfilepath);
	   ob_clean( );
	  fflush();
  }
}