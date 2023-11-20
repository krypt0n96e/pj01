
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
  var toggleSwitch = document.getElementById("toggleSwitch");
  var statusElement = document.getElementById("status");
  // Thêm sự kiện thay đổi
  toggleSwitch.addEventListener("change", function () {
    if (toggleSwitch.checked) {
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
