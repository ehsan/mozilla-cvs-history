window.onload = function ()  {
	var start = new Date();
	var lastScrollPos;
	var lastScrollTime = start;
	var max = [0, 0];
	var times = [];
	var intervalId = setInterval(function () {
		lastScrollPos = window.scrollY;
		var stepSize = 5;

		/* track the maximum scroll time */
		var now = new Date();
		var scrollTime = now - lastScrollTime;
		lastScrollTime = now;
		if (scrollTime > max[0])
			max = [scrollTime, lastScrollPos];

		times.push(scrollTime);
		window.scrollBy(0, stepSize);
		
		/* stop scrolling if we're at the end */
		if (window.scrollY == lastScrollPos) {
			clearInterval(intervalId);
			// For X11: screenX requests info from the server, so
			// this waits for the server to complete the scrolling.
			var sync = window.screenX;

			var totalDuration = new Date() - start;
			var avg = totalDuration/(window.scrollY/stepSize);
			tpRecordTime(Math.ceil(avg*1000)); // record microseconds
			//alert("took " + totalDuration + "ms\n" + window.scrollY + "avg:" + avg + "\n" + "max:" + max);
		}
	}, 10);
}
