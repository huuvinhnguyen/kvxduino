var websocket = new WebSocket('ws://' + window.location.hostname + ':81');
websocket.onmessage = function (evt) {
    var obj = JSON.parse(evt.data);
    // document.getElementById("date").innerHTML = obj.date + "/" + obj.month + "/" + obj.year;
    // document.getElementById("hour").innerHTML = obj.hour;
    // document.getElementById("minute").innerHTML = obj.minute;
    // document.getElementById("second").innerHTML = obj.second;

    // Tạo chuỗi ngày giờ dễ đọc
    var formattedDateTime = 
    obj.date.toString().padStart(2, '0') + "/" +
    obj.month.toString().padStart(2, '0') + "/" +
    obj.year + " - " +
    obj.hour.toString().padStart(2, '0') + ":" +
    obj.minute.toString().padStart(2, '0') + ":" +
    obj.second.toString().padStart(2, '0');

    document.getElementById("datetime").innerHTML = formattedDateTime;
};

// Hàm lấy thời gian từ server
function fetchTime() {
    fetch('/get_time', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        var formattedDateTime = 
        data.date.toString().padStart(2, '0') + "/" +
        data.month.toString().padStart(2, '0') + "/" +
        data.year + " - " +
        data.hour.toString().padStart(2, '0') + ":" +
        data.minute.toString().padStart(2, '0') + ":" +
        data.second.toString().padStart(2, '0');

        document.getElementById("datetime").innerHTML = formattedDateTime;
        console.log("Thời gian từ server:", formattedDateTime);
    })
    .catch(error => {
        console.error("Lỗi khi lấy thời gian từ server:", error);
    });
}

// Gọi API get_time sau khi vào web 5 giây
setTimeout(fetchTime, 1000);

// Hàm lấy thời gian từ server
function fetchDeviceInfo() {
    fetch('/get_device_info', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        
        console.log("Device information from server:", data);
    })
    .catch(error => {
        console.error("Lỗi khi lấy thời gian từ server:", error);
    });
}

// Gọi API get_time sau khi vào web 5 giây
setTimeout(fetchDeviceInfo, 3000);

document.getElementById("timeForm").addEventListener("submit", function (event) {
    event.preventDefault();

    let dateTimeValue = document.getElementById("datetimePicker").value;
    if (!dateTimeValue) {
        alert("Please select a valid date and time.");
        return;
    }

    let selectedDate = new Date(dateTimeValue);
    let timeData = {
        year: selectedDate.getFullYear(),
        month: selectedDate.getMonth() + 1,  // JavaScript tháng tính từ 0-11
        day: selectedDate.getDate(),
        hour: selectedDate.getHours(),
        minute: selectedDate.getMinutes(),
        second: selectedDate.getSeconds()
    };

    fetch('/set_time', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(timeData)
    })
        .then(response => response.json())
        .then(data => {
            document.getElementById("message").textContent = "Time updated successfully!";
            document.getElementById("message").style.color = "green";
            fetchTime(); // Cập nhật lại hiển thị thời gian
        })
        .catch(error => {
            document.getElementById("message").textContent = "Failed to update time!";
            document.getElementById("message").style.color = "red";
        });
});

function toggleRelay(isOn) {
    const relayIndex = document.getElementById("relayIndex").value;
    const data = {
        relay_index: parseInt(relayIndex),
        switch_value: isOn ? 1 : 0
    };

    fetch("/switchon", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(data)
    })
    .then(response => response.json())
    .then(data => {
        document.getElementById("relayMessage").innerText = "Relay updated successfully!";
        document.getElementById("relayMessage").style.color = "green";
    })
    .catch(error => {
        console.error("Error:", error);
        document.getElementById("relayMessage").innerText = "Failed to update relay.";
        document.getElementById("relayMessage").style.color = "red";
    });
}

function addReminder() {
    const relayIndex = parseInt(document.getElementById("relayIndex").value);
    const startTime = document.getElementById("startTime").value;
    const duration = parseInt(document.getElementById("duration").value) * 1000 * 60; // by minutes
    const repeatType = document.getElementById("repeatType").value;

    const data = {
        relay_index: relayIndex,
        reminder: {
            start_time: startTime,
            duration: duration,
            repeat_type: repeatType
        }
    };

    fetch("/add_reminder", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(data)
    })
    .then(response => response.json())
    .then(result => {
        document.getElementById("statusMessage").textContent = result.message;
    })
    .catch(error => {
        document.getElementById("statusMessage").textContent = "Lỗi: Không thể thêm hẹn giờ!";
        console.error("Lỗi:", error);
    });
}

function fetchDeviceInfo() {
    fetch('/get_device_info', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        console.log("Device Info:", data);
        displayReminders(data.relays); // Hiển thị reminders từ tất cả các relay
    })
    .catch(error => {
        console.error("Lỗi khi lấy thông tin thiết bị:", error);
    });
}

function displayReminders(relays) {
    const reminderList = document.getElementById("reminderList");
    reminderList.innerHTML = ""; // Xóa danh sách cũ

    if (relays.length === 0) {
        reminderList.innerHTML = "<p>Không có relay nào.</p>";
        return;
    }

    relays.forEach((relay, relayIndex) => {
        if (!relay.reminders || relay.reminders.length === 0) {
            return;
        }

        document.getElementById("reminderStatus").textContent = relay.is_reminders_active 

        relay.reminders.forEach((reminder, reminderIndex) => {
            if (reminder.start_time === "") return; // Bỏ qua reminders không hợp lệ

            const reminderItem = document.createElement("div");

            reminderItem.innerHTML = `
                <p><b>Relay ${relayIndex + 1} - Reminder ${reminderIndex + 1}:</b></p>
                <p>Thời gian bắt đầu: ${reminder.start_time}</p>
                <p>Thời lượng: ${reminder.duration / 60000} phút</p>
                <p>Kiểu lặp lại: ${reminder.repeat_type}</p>
            `;
            reminderList.appendChild(reminderItem);
        });
    });
}

// Gọi API để lấy danh sách reminders sau khi vào trang web 5 giây
setTimeout(fetchDeviceInfo, 3000);

function deleteAllReminders() {
    fetch("/remove_reminders", {
        method: "POST",
        headers: { "Content-Type": "application/json" }
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            document.getElementById("deleteMessage").textContent = "Xoá tất cả hẹn giờ thành công!";
            document.getElementById("deleteMessage").style.color = "green";
            displayReminders([]); // Cập nhật giao diện với danh sách trống
        } else {
            document.getElementById("deleteMessage").textContent = "Lỗi khi xoá hẹn giờ!";
            document.getElementById("deleteMessage").style.color = "red";
        }
    })
    .catch(error => {
        console.error("Error:", error);
        document.getElementById("deleteMessage").textContent = "Lỗi khi gọi API xoá hẹn giờ!";
        document.getElementById("deleteMessage").style.color = "red";
    });
}

function toggleReminders() {
    const relayIndex = parseInt(document.getElementById("relayIndex").value);
    const reminderStatusElement = document.getElementById("reminderStatus");
    const currentStatus = reminderStatusElement.textContent === "Bật"; // Kiểm tra trạng thái hiện tại
    const isRemindersActive = !currentStatus; // Đảo trạng thái hiện tại

    const data = {
        relay_index: relayIndex,
        is_reminders_active: isRemindersActive
    };

    fetch("/set_reminders_active", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(data)
    })
    .then(response => response.json())
    .then(result => {
        if (result.success) {
            reminderStatusElement.textContent = isRemindersActive ? "Bật" : "Tắt"; // Cập nhật trạng thái hiển thị
            document.getElementById("remindersStatusMessage").textContent = "Reminders đã được cập nhật!";
            document.getElementById("remindersStatusMessage").style.color = "green";
        } else {
            document.getElementById("remindersStatusMessage").textContent = "Lỗi khi cập nhật reminders!";
            document.getElementById("remindersStatusMessage").style.color = "red";
        }
    })
    .catch(error => {
        console.error("Error:", error);
        document.getElementById("remindersStatusMessage").textContent = "Lỗi khi gọi API!";
        document.getElementById("remindersStatusMessage").style.color = "red";
    });
}
