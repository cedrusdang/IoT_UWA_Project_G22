var mapOptions;
var map;

var coordinates = []
let new_coordinates = []
let lastElement

function InitMap() {
    // 设置 UWA 的经纬度
    var location = new google.maps.LatLng(-31.977, 115.816) 
    mapOptions = {
        zoom: 15,  // 缩放级别为 15，可以更清楚地查看 UWA
        center: location,
        mapTypeId: google.maps.MapTypeId.ROADMAP  // 地图类型设置为道路地图
    }
    map = new google.maps.Map(document.getElementById('map-canvas'), mapOptions)

    var all_overlays = [];
    var selectedShape;
    var drawingManager = new google.maps.drawing.DrawingManager({
        drawingControlOptions: {
            position: google.maps.ControlPosition.TOP_CENTER,
            drawingModes: [
                google.maps.drawing.OverlayType.POLYGON  // 仅启用多边形绘制工具
            ]
        },
        polygonOptions: {
            clickable: true,
            draggable: false,
            editable: true,
            fillColor: '#ADFF2F',
            fillOpacity: 0.5
        }
    });

    function clearSelection() {
        if (selectedShape) {
            selectedShape.setEditable(false);
            selectedShape = null;
        }
    }

    // 停止绘图工具
    function stopDrawing() {
        drawingManager.setMap(null);
    }

    // 选择形状时设置编辑功能
    function setSelection(shape) {
        clearSelection();
        stopDrawing()
        selectedShape = shape;
        shape.setEditable(true);
    }

    // 删除选中的多边形
    function deleteSelectedShape() {
        if (selectedShape) {
            selectedShape.setMap(null);
            drawingManager.setMap(map);
            coordinates.splice(0, coordinates.length)
            document.getElementById('info').innerHTML = ""
        }
    }

    function CenterControl(controlDiv, map) {
        var controlUI = document.createElement('div');
        controlUI.style.backgroundColor = '#fff';
        controlUI.style.border = '2px solid #fff';
        controlUI.style.borderRadius = '3px';
        controlUI.style.boxShadow = '0 2px 6px rgba(0,0,0,.3)';
        controlUI.style.cursor = 'pointer';
        controlUI.style.marginBottom = '22px';
        controlUI.style.textAlign = 'center';
        controlUI.title = 'Select to delete the shape';
        controlDiv.appendChild(controlUI);

        var controlText = document.createElement('div');
        controlText.style.color = 'rgb(25,25,25)';
        controlText.style.fontFamily = 'Roboto,Arial,sans-serif';
        controlText.style.fontSize = '16px';
        controlText.style.lineHeight = '38px';
        controlText.style.paddingLeft = '5px';
        controlText.style.paddingRight = '5px';
        controlText.innerHTML = 'Delete Selected Area';
        controlUI.appendChild(controlText);

        // 处理删除多边形的点击事件
        controlUI.addEventListener('click', function () {
            deleteSelectedShape();
        });
    }

    drawingManager.setMap(map);

    var getPolygonCoords = function (newShape) {
        coordinates.splice(0, coordinates.length)
        var len = newShape.getPath().getLength();

        for (var i = 0; i < len; i++) {
            coordinates.push(newShape.getPath().getAt(i).toUrlValue(6))
        }
        document.getElementById('info').innerHTML = coordinates.join('<br>');
    }

    // 处理多边形绘制完成事件
    google.maps.event.addListener(drawingManager, 'polygoncomplete', function (event) {
        getPolygonCoords(event);

        google.maps.event.addListener(event.getPath(), 'insert_at', function () {
            getPolygonCoords(event);
        });

        google.maps.event.addListener(event.getPath(), 'set_at', function () {
            getPolygonCoords(event);
        });
    });

    // 处理绘制形状的完成事件
    google.maps.event.addListener(drawingManager, 'overlaycomplete', function (event) {
        all_overlays.push(event);
        if (event.type !== google.maps.drawing.OverlayType.MARKER) {
            drawingManager.setDrawingMode(null);

            var newShape = event.overlay;
            newShape.type = event.type;
            google.maps.event.addListener(newShape, 'click', function () {
                setSelection(newShape);
            });
            setSelection(newShape);
        }
    })

    var centerControlDiv = document.createElement('div');
    var centerControl = new CenterControl(centerControlDiv, map);

    centerControlDiv.index = 1;
    map.controls[google.maps.ControlPosition.BOTTOM_CENTER].push(centerControlDiv);
}

InitMap();

