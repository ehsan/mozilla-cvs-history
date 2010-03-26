function doScroll()  {
	
	var scrollTarget = document.getElementById("ifrm").contentWindow;
	//scrollTarget.onload = function () {
	var start = new Date();
	var lastScrollPos;
	var lastScrollTime = start;
	var max = [0, 0];
	var times = [];
	var intervalId = setInterval(function () {
		lastScrollPos = scrollTarget.scrollY;
		var stepSize = 30;

		/* track the maximum scroll time */
		var now = new Date();
		var scrollTime = now - lastScrollTime;
		lastScrollTime = now;
		if (scrollTime > max[0])
			max = [scrollTime, lastScrollPos];

		times.push(scrollTime);
		scrollTarget.scrollBy(0, stepSize);
		
		/* stop scrolling if we're at the end */
		if (scrollTarget.scrollY == lastScrollPos) {
			clearInterval(intervalId);
			var totalDuration = new Date() - start;
			var avg = totalDuration/(scrollTarget.scrollY/stepSize);
			tpRecordTime(Math.ceil(avg*1000)); // record microseconds
		}
	}, 10);
	//}
}
