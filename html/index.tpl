<html>
<head><title>%app% (%version%)</title>
<link rel="stylesheet" type="text/css" href="style.css">
</head>
<body>
<div id="main">
<h1>It Works!!!</h1>
<p>
If you see this, it means the tiny li'l website in your ESP8266 does actually work. Fyi, this page has
been loaded <b>%counter%</b> times.
<ul class=button>
<li><a href="/wifi">WiFi Settings</a></li>
<li><a href="/led.tpl">LED Control</a></li>
<li><a href="/flash.bin">Read Flash</a></li>
<li><a href="http://spritesmods.com/?f=esphttpd">Sprites Website</a></li>
</ul>
<div id="maintxt">
<ul>
<li>If you haven't connected this device to your WLAN network now, you can <a href="/wifi">do so.</a></li>
<li>You can also control the <a href="led.tpl">LED</a>.</li>
<li>You can download the raw <a href="flash.bin">contents</a> of the SPI flash rom</li>
<li>And because I can, here's a link to my <a href="http://spritesmods.com/?f=esphttpd">website</a></ul>
</ul>
</div>
</p>
Sorry for the cats.  I am impatient and got tired of uploading them all the G..D... time.
</div>
<br>
<div id="footer">%app% (%version%) running sdk_v%sdk% - Boot Partition %boot%<div>
</body></html>
