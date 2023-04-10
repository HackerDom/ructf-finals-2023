let VIDEO_STATUS = 0;

const loader = document.getElementById('loader');
const content = document.getElementById('content');
const background = document.getElementById('background');

const innerVideo = document.getElementById('inner-video');
const outerVideo = document.getElementById('outer-video');

innerVideo.onloadeddata = function() {
    VIDEO_STATUS |= 0b01;
};

outerVideo.onloadeddata = function() {
    VIDEO_STATUS |= 0b10;
};

const setContentWrapper = function() {
    const ratio = 1280 / 534;

    const width = Math.max(document.documentElement.clientWidth || 0, window.innerWidth || 0);
    const height = Math.max(document.documentElement.clientHeight || 0, window.innerHeight || 0);

    if (width > ratio * height) {
        content.className = "content-wrapper content-wrapper-vertical";
    } else {
        content.className = "content-wrapper content-wrapper-horizontal";
    }
};

window.addEventListener('resize', setContentWrapper);
setContentWrapper();

const updateStatus = function() {
    if (VIDEO_STATUS & 0b11 === 0b11) {
        innerVideo.play();
        outerVideo.play();

        setTimeout(function() {
            loader.hidden = true;

            background.hidden = false;
            content.hidden = false;
        }, 500);

        return;
    }

    setTimeout(updateStatus, 100);
};

updateStatus();
