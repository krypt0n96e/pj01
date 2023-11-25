document.addEventListener("DOMContentLoaded", function () {

  // Function to update the table with new data
  function updateTable(topEntries) {
    var tableBody = $('#data-table tbody');

    tableBody.empty(); // Clear existing content

    topEntries.forEach(function (entry) {
      var row = '<tr>' +
        '<td>' + entry.id + '</td>' +
        '<td>' + entry.data + '</td>' +
        '<td>' + entry.date + '</td>' +
        '<td>' + entry.device_id + '</td>' +
        '<td><button onclick="deleteData(' + entry.id + ')" class="btn btn-danger">Delete</button></td>' +
        '</tr>';

      tableBody.append(row);
    });
    console.log('Table updated successfully.'); // Log success message
  }

  // Func making chart
  // Declare a global variable to store the chart instance
  // Declare a global variable to store the chart instance
  var myChart;

  // Function to draw the line chart
  function drawLineChart(labels, data) {
    var ctx = document.getElementById('myChart').getContext('2d');

    // Check if the chart instance exists
    if (myChart) {
      // If it does, update the chart data and labels
      myChart.data.labels = labels;
      myChart.data.datasets[0].data = data;
      myChart.update(); // Update the chart
    } else {
      // If the chart instance doesn't exist, create a new chart
      console.log('Labels:', labels);
      console.log('Data:', data);

      myChart = new Chart(ctx, {
        type: 'line',
        data: {
          labels: labels,
          datasets: [{
            label: 'Data Trends',
            data: data,
            borderColor: 'rgba(75, 192, 192, 1)',
            borderWidth: 1,
            fill: true,
            pointRadius: 3,
            pointBackgroundColor: 'rgba(75, 192, 192, 1)',
          }]
        },
        options: {
          scales: {
            x: {
              type: 'time',
              time: {
                unit: 'millisecond', // Set the unit to 'millisecond'
                tooltipFormat: 'SSS [ms]', // Format for milliseconds
              },
            },
            y: {
              min: 0,
              max: 5000,
            }
          }
        }
      });
    }
  }



  // AJAX request to fetch data and update the table
  function fetchDataAndPopulateTable() {
    $.ajax({
      url: '/esp',
      method: 'GET',
      success: function (dataList) {
        
        // Sort dataList based on the date in descending order
        dataList.sort(function (a, b) {
          return b.id - a.id;
        });

        // Only display the top 100 entries
        var topEntries = dataList.slice(0, 50);

        updateTable(topEntries);
        // Assuming dataList is your array of data objects
        var labels = topEntries.map(entry => new Date(entry.date));
        var values = topEntries.map(entry => Number(entry.data));

        // Draw the line chart
        drawLineChart(labels, values);

      },
      error: function (error) {
        console.error('Error fetching data:', error);
      }
    });
  }

  // Call the function to fetch data and populate the table on page load
  fetchDataAndPopulateTable();

  // Optionally, set up a timer to update the table periodically
  setInterval(fetchDataAndPopulateTable, 1000); // Update every 500 milliseconds
});
// //phan giup autoreload page
// document.addEventListener('DOMContentLoaded', function () {
//   var reloadCheck = document.getElementById('reloadCheck');

//   function reloadPage() {
//     location.href = location.href.split("?")[0] + "?t=" + new Date().getTime();
//   }

//   function stopAutoReload() {
//     clearInterval(reloadInterval);
//   }

//   function startAutoReload() {
//     reloadInterval = setInterval(function () {
//       reloadPage();
//     }, 500);
//   }
//   // Load state from localStorage
//   var isAutoReloadEnabled = localStorage.getItem('isAutoReloadEnabled') === 'true';

//   // Set initial state of the toggle switch
//   reloadCheck.checked = isAutoReloadEnabled;

//   // Add an event listener to the toggle switch
//   reloadCheck.addEventListener('change', function () {
//     if (reloadCheck.checked) {
//       startAutoReload();
//     } else {
//       stopAutoReload();
//     }
//     // Save state to localStorage
//     localStorage.setItem('isAutoReloadEnabled', reloadCheck.checked);
//   });

//   // Start auto-reload if it was enabled
//   if (isAutoReloadEnabled) {
//     startAutoReload();
//   }
// });

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
