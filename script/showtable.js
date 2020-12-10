var prtcols;
var pgidx = 0;
var dataobj;

function showtable() {
  let result = document.querySelector('#result');
  let users = document.querySelector('#users');
  let table = document.querySelector('#tblname');
  let cmd = document.querySelector('#cmd');

  if (table.value == "") {
    alert("Please input table name!!!");
    return;
  }

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
      dataobj = JSON.parse(this.responseText);
      let ohead = dataobj.h;
      if (prtcols == 1) {
        fillHead(ohead);
        prtcols = 0;
      }
      fillRows();
      let loadbtn = document.querySelector('.load_btn');
      loadbtn.disabled = true;
      //loadbtn.style.visibility = "hidden";
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

function padding(num, length) {
  let len = (num + "").length;
  let diff = length - len + 1; // pad number of length 0s
  if (diff > 0) {
    return Array(diff).join("0") + num;
  }
  return num;
}

function fillRows() {
  let oTable = document.querySelector('#users');
  let tBodies = oTable.tBodies;
  let tbody = tBodies[0];
  let rowNum = 0;
  let odata = dataobj.d;
  let totalRows = Object.keys(odata).length;
  let pagesz = document.querySelector('#pagesz').value;

  do {
    let pgRowNum = pagesz*pgidx + rowNum;
    if (pgRowNum >= totalRows) return;
    let row = "r" + padding(pgRowNum, 3);
    let tr = tbody.insertRow(rowNum);
    let td0 = tr.insertCell(0);
    td0.innerHTML = "<div class='idx'>" + (pgRowNum+1) + "</div>";
    // i - col number
    for (i = 0; i < odata[row].length; i++) {
      let td = tr.insertCell(i+1);
      td.innerHTML = "<div class='tdata' contenteditable='true'>" +
                     odata[row][i] +
                     "</div>";
    }
    rowNum++;
  } while (rowNum < pagesz);
}

function delRows() {
  let oTable = document.querySelector('#users');
  let tBodies = oTable.tBodies;
  let tbody = tBodies[0];

  if (tbody !== "undefined") {
    while (tbody.hasChildNodes()) {
      tbody.removeChild(tbody.lastChild);
    }
  }
}

function nextpage() {
  let pagesz = document.querySelector('#pagesz').value;
  let odata = dataobj.d;
  let totalRows = Object.keys(odata).length;
  let totalpgs = totalRows/pagesz;
  let total = Math.round(totalpgs);
  if (total == totalpgs) {
    total--;
  }
  else {
    total = Math.round(totalpgs-0.5);
  }
  if (pgidx < total) {
    delRows();
    pgidx++;
    fillRows();
  }
}

function prevpage() {
  let pagesz = document.querySelector('#pagesz').value;
  let odata = dataobj.d;
  let totalRows = Object.keys(odata).length;
  let totalpgs = totalRows/pagesz;
  if (pgidx > 0) {
    delRows();
    pgidx--;
    fillRows();
  }
}

function topage() {
  if (event.keyCode == 13) {
    let pagesz = document.querySelector('#pagesz').value;
    let odata = dataobj.d;
    let totalRows = Object.keys(odata).length;
    let totalpgs = totalRows/pagesz;
    if (pgidx >= 0) {
      delRows();
      pgidx = document.querySelector('#pgnum').value-1;
      fillRows();
    }
  }
}
