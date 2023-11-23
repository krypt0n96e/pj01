//phan giup autoreload page
document.addEventListener('DOMContentLoaded', function () {
  var reloadCheck = document.getElementById('reloadCheck');

  function reloadPage() {
    location.href = location.href.split("?")[0] + "?t=" + new Date().getTime();
  }

  function stopAutoReload() {
    clearInterval(reloadInterval);
  }

  function startAutoReload() {
    reloadInterval = setInterval(function () {
      reloadPage();
    }, 9000);
  }
  // Load state from localStorage
  var isAutoReloadEnabled = localStorage.getItem('isAutoReloadEnabled') === 'true';

  // Set initial state of the toggle switch
  reloadCheck.checked = isAutoReloadEnabled;

  // Add an event listener to the toggle switch
  reloadCheck.addEventListener('change', function () {
    if (reloadCheck.checked) {
      startAutoReload();
    } else {
      stopAutoReload();
    }
    // Save state to localStorage
    localStorage.setItem('isAutoReloadEnabled', reloadCheck.checked);
  });

  // Start auto-reload if it was enabled
  if (isAutoReloadEnabled) {
    startAutoReload();
  }
});

//-------------



function closeAlert(button) {
  // Tìm ra phần tử cha của nút đóng (button)
  var alertDiv = button.closest('.alert');

  // Ẩn phần tử cha (đó là thông báo)
  if (alertDiv) {
    alertDiv.style.display = 'none';
  }
}

function toggleChange(log) {
  // Lấy tham chiếu đến phần tử checkbox và phần tử hiển thị trạng thái
  var devideSw = document.getElementById("devideSw");
  // Thêm sự kiện thay đổi
  devideSw.addEventListener("change", function () {
    if (devideSw.checked) {
      statusChange(1);
    } else {
      statusChange(0);
    }
  });
}



function deleteData(dataId) {
  fetch("/delete-data", {
    method: "POST",
    body: JSON.stringify({ dataId: dataId }),
  }).then((_res) => {
    window.location.href = "/";
  });
}

function statusChange(log) {
  fetch("/status-change", {
    method: "POST",
    body: JSON.stringify({ log: log }),
  }).then((_res) => {
    window.location.href = "/";
  });
}
