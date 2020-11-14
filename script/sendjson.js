function sendJSON() {
  let result = document.querySelector('.result');
  let svc = document.querySelector('#svc');
  let action= document.querySelector('#action');
  let first_name = document.querySelector('#first_name');
  let last_name = document.querySelector('#last_name');
  let email = document.querySelector('#email');

  // Creating a XHR object
  let xhr = new XMLHttpRequest();
  let url = "sendjson.html";

  // open a connection
  xhr.open("POST", url, true);

  // Set the request header i.e. which type of content you are sending
  xhr.setRequestHeader("Content-Type", "application/json");

  // Create a state change callback
  xhr.onreadystatechange = function () {
    if (xhr.readyState === 4 && xhr.status === 200) {
      // Print received data from server
      result.innerHTML = this.responseText;
    }
  };

  // Converting JSON data to string
  var data = JSON.stringify({"svc": svc.value,
                             "action": action.value,
                             "first_name": first_name.value,
                             "last_name": last_name.value,
                             "email": email.value});
  // Sending data with the request
  xhr.send(data);
}
