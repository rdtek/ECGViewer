var adx_ecg = {

    DPI                : 96.0,
    MM_PER_BIG_SQUARE  : 25.0,
    MS_PER_BIG_SQUARE  : 1000,
    MM_PER_INCH        : 25.4, //25.4 millimetres = 1 inch.
    BIG_GRID_COLOR     : "#F08080", //rgb(240, 128, 128);
    SMALL_GRID_COLOR   : "#FFC0C0", //rgb(255, 192, 192);
    SIGNAL_COLOR       : "#000033",
    SAMPLE_FREQUENCY   : 500,
    SAMPLE_RESOLUTION  : 0.005,
    TRACKS_PER_PAGE    : 5,
    SCALE_FACTOR       : 1,
    MIN_HEIGHT         : 800,
    PADDING_X          : 50,

    SignalData         : [],
    AnalysisData       : [],

    ECGCanvas : function () { return $("#ECGCanvas"); },

    SetResizeHandler : function (callback, timeout) {
        var timer_id = undefined;
        window.addEventListener("resize", function () {
            if (timer_id != undefined) {
                clearTimeout(timer_id);
                timer_id = undefined;
            }
            timer_id = setTimeout(function () {
                timer_id = undefined;
                callback();
            }, timeout);
        });
    },

    InitCanvas : function () { 
        var canvas = this.ECGCanvas();
        var parent = canvas.parent();
		
		parent.find(".ECGChooseFile").change(function() {
			alert("change");
			var input = event.target;
			var reader = new FileReader();
			reader.onload = function(){
				var text = reader.result;
				//TODO: load data into SignalData array for drawing
				console.log(reader.result.substring(0, 200));
			};
			reader.readAsText(input.files[0]);
		});
		
		parent.find(".BtnOpenECG").off("click").on( "click", function() {
			var fileApi = !!(window.File && window.FileReader && window.FileList && window.Blob);
			if(fileApi)
				parent.find(".ECGChooseFile").click();
			else
				console.log("File API not supported.");
		});
		
        var context = canvas.get(0).getContext("2d");
        context.canvas.width = parent.outerWidth() 
			- (canvas.css("marginLeft").replace('px', '') * 2) - 5;
        context.canvas.height = parent.outerHeight() 
			- (canvas.css("marginTop").replace('px', '') * 2) - 5;
        this.Paint();
    },

    CanvasContext: function () {
        var canvas = this.ECGCanvas();
        var context = canvas.get(0).getContext("2d");
        context.lineCap = "round";
        return context;
    },

	// Name:    EcgBigSquarePx
    // Desc:    Get the width and height of big ECG grid square representing 1 second
    // Returns: Width and height of big grid square in pixel units
    EcgBigSquarePx : function () {
        return this.ScaleFactor() * Math.round(this.MM_PER_BIG_SQUARE * this.DPI / this.MM_PER_INCH);
    },

	// Name:    EcgSmallSquarePx
    // Desc:    Get the width and height of small ECG grid square representing 1/5 second
    // Returns: Width and height of small square in pixel units
    EcgSmallSquarePx : function () {
        return this.EcgBigSquarePx() / 5;
    },

	// Name:    TrackWidthPx
    // Desc:    Calculates width of signal track taking account of padding or margins
    // Returns: Width of track in pixel units as integer
    TrackWidthPx : function () {
        var me = this;
        return me.ECGCanvas().outerWidth() - 2 * me.PADDING_X;
    },

    TrackYOffset : function () {
        return 2.5 * this.EcgBigSquarePx();
    },

    ScaleFactor : function (scaleFactor) {
        var me = this;
        if (arguments.length >= 1)
            me.SCALE_FACTOR = scaleFactor;
        return me.SCALE_FACTOR;
    },

	
    // Name:    PointsPerTrack
    // Desc:    Calculates how many points fit in one signal track
    // Returns: Whole number of points for one track
    PointsPerTrack : function () {
        var me = this;
        return Math.round(me.TrackWidthPx() / me.EcgBigSquarePx() * me.SAMPLE_FREQUENCY);
    },

    IncreaseScaleFactor : function (increment) {
        var me = this;
        if (arguments.length >= 1)
            me.SCALE_FACTOR += increment;
        me.Paint();
    },

    DecreaseScaleFactor : function (increment) {
        var me = this;
        if (arguments.length >= 1)
            me.SCALE_FACTOR -= increment;
        me.Paint();
    },

    Clear : function () {
        var me = this;
        me.SignalData = [];
        me.AnalysisData = [];
        me.Paint();
    },

    Paint: function () {
        var me = this;
        me.SetCanvasHeight();
        me.DrawGrid();
        me.DrawSignal();
        me.DrawAnnotations();
    },

    SetCanvasHeight : function(){
        var me = this;
        var ctx = me.CanvasContext();
        var totalTracks = Math.ceil(me.SignalData.length / me.PointsPerTrack());
        var calculatedHeight = totalTracks * me.TrackYOffset() + me.TrackYOffset();
        ctx.canvas.height = calculatedHeight > me.MIN_HEIGHT ? calculatedHeight : me.MIN_HEIGHT;
    },

	// Name:   DrawLine
    // Desc:   Draws single line onto HTML canvas, used to draw grid lines and signal waves.
    DrawLine : function (x1, y1, x2, y2, penWidth, penColor) {
        var ctx = this.CanvasContext();
        ctx.lineWidth = penWidth;
        ctx.beginPath();
        ctx.strokeStyle = penColor;
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
    },

    // Name:   DrawGrid
    // Desc:   Draws grid squares indicating time along horizontal and voltage in vertical
    DrawGrid : function() {

        var me = this;
        var ctx = me.CanvasContext();

        ctx.fillStyle = "#fff";
        ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);

        var DrawGridLines = function (square, maxWidth, maxHeight, color) {
            var x = -square, y = -square;
            while ((x += square) < maxWidth) me.DrawLine(x, 0, x, maxHeight, 1, color);
            while ((y += square) < maxHeight) me.DrawLine(0, y, maxWidth, y, 1, color);
        }

        DrawGridLines(me.EcgSmallSquarePx(), ctx.canvas.width, ctx.canvas.height, me.SMALL_GRID_COLOR);
        DrawGridLines(me.EcgBigSquarePx(), ctx.canvas.width, ctx.canvas.height, me.BIG_GRID_COLOR);
    },

	// Name:   DrawSignal
    // Desc:   Draws rows of signal wave lines overlaid on ECG grid
    DrawSignal : function () {

        var me = this;
        var ctx = me.CanvasContext();
        var signalData = me.SignalData;
        var yOffset = me.TrackYOffset() / 2;
        var  i = 0, j = 0, xPos = me.PADDING_X, yPos = 0;

        for (i = 0, j = 0; j < signalData.length - 1; i += 1, j += 1) {

            var x1 = xPos;
            var y1 = yOffset + me.ScaleSignalYToPixels(signalData[j] * -1);
            var x2 = me.PADDING_X + me.ScaleSignalXToPixels(i + 1);
            var y2 = yOffset + me.ScaleSignalYToPixels(signalData[j + 1] * -1);

            me.DrawLine(x1, y1, x2, y2, 1, me.SIGNAL_COLOR);

            xPos = x2;
            yPos = y2;

            if (i == (me.PointsPerTrack() - 1)) {
                i = 0;
                xPos = me.PADDING_X;
                yOffset += me.TrackYOffset();
            }
        }
    },

    ScaleSignalXToPixels : function (sampleIndex) {
        var xPixels = sampleIndex / this.SAMPLE_FREQUENCY * this.EcgBigSquarePx();
        return Math.round(xPixels);
    },

	// Name:    ScaleSignalXToPixels
    // Desc:    Creates a X coordinate based on grid scale so we can draw the signal point
    // Param:   sampleIndex - the index of the sample in the signal samples array
    // Returns: X pixel coordinate as integer
    ScaleSignalYToPixels : function(sample) {
        var yPixels = sample * this.SAMPLE_RESOLUTION * this.EcgBigSquarePx();
        return Math.round(yPixels);
    },

    DrawAnnotations : function(){

        if (this.AnalysisData == null || this.AnalysisData.Beats == null) return;

        var ctx = this.CanvasContext();
        ctx.font = "11px Arial";

        var annotations = this.GetAnnotations(1);
        
        for (var i = 0; i < annotations.length; i++)
        {
            var a = annotations[i];
            var p = a.point;
            ctx.fillStyle = 'rgba(225,225,225,0.9)';
            ctx.fillRect(p.x - 8, p.y - 12, 14, 16);
            ctx.fillStyle = '#000000';
            ctx.fillText(a.label, p.x - 5, p.y);
        }
    },
    
    TimeToX : function (timeMs) {
        var me = this;
        var x = timeMs / me.MS_PER_BIG_SQUARE * me.EcgBigSquarePx();
        var trackNum = Math.floor(x / me.TrackWidthPx());
        return x % me.TrackWidthPx();
    },

    TrackToY : function (trackNum) {
        var me = this;
        var y = (trackNum % me.TRACKS_PER_PAGE + 1) * me.TrackYOffset();
        return y + 3 * me.EcgSmallSquarePx();
    },

    GetAnnotationPosition : function (timeMs) {
        var me = this;
        var x = 0, y = 0, trackNum = 0;

        x = timeMs / me.MS_PER_BIG_SQUARE * me.EcgBigSquarePx();
        trackNum = Math.floor(x / me.TrackWidthPx());
        x = x % me.TrackWidthPx();

        y = (trackNum % me.TRACKS_PER_PAGE + 1) * me.TrackYOffset();
        y = y + 3 * me.EcgSmallSquarePx();

        return { x: x, y: y };
    },

    GetAnnotations : function(pageNum){
        
        var me = this;
        var annotations = [];
        var beats = me.AnalysisData.Beats;

        var timeMsPerPage = ((me.TrackWidthPx() * me.TRACKS_PER_PAGE) / me.EcgBigSquarePx()) * 1000;
        var pageStartTime = (pageNum - 1) * timeMsPerPage;
        var pageEndTime = pageStartTime + timeMsPerPage;

        for (var i = 0; i < beats.length; i++)
        {
            var beat = beats[i];
            if (beat.Time >= pageStartTime && beat.Time < pageEndTime) {

                annotations.push({
                    label: beat.Label,
                    time: beat.Time,
                    point: me.GetAnnotationPosition(beat.Time)
                });
            }
        }   

        return annotations;
    }
}

window.onload = function () {
    adx_ecg.SetResizeHandler(function () {
        adx_ecg.InitCanvas();
    }, 100);
    adx_ecg.InitCanvas();
}