document.addEventListener("DOMContentLoaded", function () {

  // Function to update the table with new data
  function updateTable(topEntries) {
    var tableBody = $('#data-table tbody');

    tableBody.empty(); // Clear existing content

    topEntries.forEach(function (entry) {
      // Add a class to the second column for styling
      var rowData = '<td class="multiline">' + entry.data + '</td>';

      var row = '<tr>' +
        '<td>' + entry.id + '</td>' +
        rowData +
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
  
    // Calculate min and max values for the x-axis
    var xMin = Math.min(...labels);
    var xMax = Math.max(...labels);
  
    // Check if the chart instance exists
    if (myChart) {
      // If it does, update the chart data and labels
      myChart.data.labels = labels;
      myChart.data.datasets[0].data = data;
      myChart.options.scales.x.min = xMin;
      myChart.options.scales.x.max = xMax;
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
            borderColor: 'rgba(32, 135, 71, 1)',
            borderWidth: 1,
            fill: true,
            pointRadius: 2,
            pointBackgroundColor: 'rgba(32, 135, 71, 1)',
          }]
        },
        options: {
          scales: {
            x: {
              type: 'linear',
              min: xMin,
              max: xMax,
            },
            y: {
              min: 0,
              max: 4100,
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
      success: function (rawData) {



        // Sort rawData based on the date in descending order
        rawData.sort(function (a, b) {
          return b.id - a.id;
        });

        // Only display the top 100 entries
        var topTable = rawData.slice(0, 50);
        var topChart = rawData.slice(0, 5);

        // Parse and process the data
        const parsedData = topChart.map(entry => {
          const items = entry.data.split('?').filter(Boolean);
          const data = items.map(item => {
            const [time, value] = item.split('&');
            return { time, value: parseInt(value) }; // Convert value to integer
          });
          return data;
        });

        // Flatten the array of arrays
        const flatData = parsedData.reduce((acc, val) => acc.concat(val), []);
        // Sort the flattened data based on time from smallest to largest
        flatData.sort((a, b) => a.time - b.time);

        updateTable(topTable);
        // Assuming rawData is your array of data objects
        var labels = flatData.map(entry => entry.time);
        var values = flatData.map(entry => entry.value);

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

function deleteAllData() {
  fetch("/delete-all", {
    method: "POST",
    body: "200",
  }).then((_res) => {
    window.location.href = "/";
  });
}

function exportAllData() {
  // Assuming you want to send "200" as a query parameter
  const queryParams = new URLSearchParams({ data: "200" });

  fetch(`/export?${queryParams}`, {
    method: "GET",
  }).then((_res) => {
    // Redirect to the root ("/") after the request is complete
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
