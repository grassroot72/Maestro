var prtcols;

function showtable() {
  let result = document.querySelector('#result');
  let users = document.querySelector('#users');
  let table = document.querySelector('#tblname');
  let cmd = document.querySelector('#cmd');

  if (prtcols == undefined) {
    prtcols = 1;
  }

  let obj = {"table":table.value,
             "cmd":cmd.value,
             "prtcols":prtcols};

  // Creating a XHR object
  let xhr = new XMLHttpRequest();
  let url = "showtable.html";

  // open a connection
  xhr.open("POST", url, true);
  // Set the request header i.e. which type of content you are sending
  xhr.setRequestHeader("Content-Type", "application/json");

  // Create a state change callback
  xhr.onreadystatechange = function () {
    if (xhr.readyState === 4 && xhr.status === 200) {
      // Print received data from server
      let oData = JSON.parse(this.responseText);
      let ohead = oData.h;
      let odata = oData.d;
      if (prtcols == 1) {
        fillHead(ohead);
        prtcols = 0;
      }
      fillRows(odata);
    }
  };

  // Converting JSON data to string
  var data = JSON.stringify(obj);
  // Sending data with the request
  xhr.send(data);
}

function fillHead(oHead) {
  let oTable = document.querySelector('#users');
  let thead = oTable.tHead;
  let tr = thead.insertRow(0);
  let th0 = tr.insertCell(0);
  th0.innerHTML = "&nbsp;&nbsp;&nbsp;";
  for (i = 0; i < oHead.hd.length; i++) {
    let th = tr.insertCell(i+1);
    th.innerHTML = "<div class='thead'>" + oHead.hd[i] + "</div>";
  }
}

function fillRows(oData) {
  let oTable = document.querySelector('#users');
  let tBodies = oTable.tBodies;
  let tbody = tBodies[0];
  let rowlen = tbody.rows.length-1;

  for (row in oData) {
    let tr = tbody.insertRow(rowlen);
    let td0 = tr.insertCell(0);
    td0.innerHTML = "<div class='idx'>" + rowlen + "</div>";
    // i - col number
    for (i = 0; i < oData[row].length; i++) {
      let td = tr.insertCell(i+1);
      td.innerHTML = "<div class='tdata' contenteditable='false'>" +
                     oData[row][i] +
                     "</div>";
    }
    rowlen++;
  }
}
