window.onload = function ()  {
		var target = document.getElementById("entries");
		var start = new Date();
		var lastScrollPos;
		var lastScrollTime = start;
		var max = [0, 0];
		var times = [];
		var intervalId = setInterval(function () {
			lastScrollPos = target.scrollTop;
			var stepSize = 5;
			/* track the maximum scroll time */
			var now = new Date();
			var scrollTime = now - lastScrollTime;
			lastScrollTime = now;
			if (scrollTime > max[0])
				max = [scrollTime, lastScrollPos];

			times.push(scrollTime);
			target.scrollTop += stepSize;
			
			/* stop scrolling if we're at the end */
			if (target.scrollTop == lastScrollPos) {
				clearInterval(intervalId);
				// For X11: screenX requests info from the
				// server, so this waits for the server to
				// complete the scrolling.
				var sync = window.screenX;

				var totalDuration = new Date() - start;
				var avg = totalDuration/(target.scrollTop/stepSize);
				tpRecordTime(Math.ceil(avg*1000)); // record microseconds
			}
		}, 10);
};
