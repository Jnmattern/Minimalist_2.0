<!DOCTYPE html>
<html>
        <head>
                <title>Clipped</title>
                <meta charset='utf-8'>
                <meta name='viewport' content='width=device-width, initial-scale=1'>
                <link rel='stylesheet' href='http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.css' />
                <script src='http://code.jquery.com/jquery-1.9.1.min.js'></script>
                <script src='http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.js'></script>
                <style>
                        .ui-header .ui-title { margin-left: 1em; margin-right: 1em; text-overflow: clip; }
                </style>
        </head>
		<body>
<div data-role="page" id="page1">
    <div data-theme="a" data-role="header" data-position="fixed">
        <h3>
            Minimalist Configuration
        </h3>
        <div class="ui-grid-a">
            <div class="ui-block-a">
                <input id="cancel" type="submit" data-theme="c" data-icon="delete" data-iconpos="left"
                value="Cancel" data-mini="true">
            </div>
            <div class="ui-block-b">
                <input id="save" type="submit" data-theme="b" data-icon="check" data-iconpos="right"
                value="Save" data-mini="true">
            </div>
        </div>
    </div>
    <div data-role="content">

<div id="displaymode" data-role="fieldcontain">
<fieldset data-role="controlgroup" data-type="vertical" data-mini="true">
<legend>Display Mode</legend>

<?php
	if (!isset($_GET['displaymode'])) {
		$displaymode = 0; // Default to Sunday
	} else {
		$displaymode = $_GET['displaymode'];
	}
	
	$s = array( "", "", "" );
	$s[$displaymode] = " checked";
	
	echo '<input id="format1" name="displaymode" value="0" data-theme="" type="radio"' . $s[0] . '><label for="format1">Hours & Minutes<br/><img src="Minimalist-HourAndMinutes.png" width="72" height="84"></label>';
	echo '<input id="format2" name="displaymode" value="1" data-theme="" type="radio"' . $s[1] . '><label for="format2">Hours at minute hand<br/><img src="Minimalist-HourAtMinuteHand.png" width="72" height="84"></label>';
	echo '<input id="format3" name="displaymode" value="2" data-theme="" type="radio"' . $s[2] . '><label for="format3">Minutes at hour hand<br/><img src="Minimalist-MinutesAtHourHand.png" width="72" height="84"></label>';
	?>
</fieldset>
</div>


<div data-role="fieldcontain">
            <label for="seconds">
                Show seconds
		</label>
            <select name="seconds" id="seconds" data-theme="" data-role="slider" data-mini="true">
<?php
	if (!isset($_GET['seconds'])) {
		$seconds = 1;
	} else {
		$seconds = $_GET['seconds'];
	}
	
	if ($seconds == 0) {
		$s1 = " selected";
		$s2 = "";
	} else {
		$s1 = "";
		$s2 = " selected";
	}
	echo '<option value="0"' . $s1 .'>Off</option><option value="1"' . $s2 . '>On</option>';
?>
            </select>
        </div>

</select>
</div>

</div>

    <script>
      function saveOptions() {
        var options = {
			'displaymode': parseInt($("input[name=displaymode]:checked").val(), 10),
			'seconds': parseInt($("#seconds").val(), 10)
        }
        return options;
      }

      $().ready(function() {
        $("#cancel").click(function() {
          console.log("Cancel");
          document.location = "pebblejs://close#";
        });

        $("#save").click(function() {
          console.log("Submit");
          
          var location = "pebblejs://close#" + encodeURIComponent(JSON.stringify(saveOptions()));
          console.log("Close: " + location);
          console.log(location);
          document.location = location;
        });

      });
    </script>
</body>
</html>
