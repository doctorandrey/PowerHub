
let ws;
function sendWSCommand(cmd) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    console.log("Cmd:", cmd);
    ws.send(JSON.stringify({ command: cmd }));
  }
}

async function loadChannelInfo() {
  const res = await fetch('/api');
  const data = await res.json();
  const container = document.getElementById('channelContainer');
  container.innerHTML = '';

  data.commands.forEach(cmd => {
    const div = document.createElement('div');
    div.classList.add('card');

    const title = document.createElement('p');
    title.classList.add('card-title');
    title.innerText = `${cmd.command} (${cmd.type})`;

    const val = document.createElement('p');
    val.classList.add('reading');
    val.innerText = `Current: ${cmd.value}`;

    const control = document.createElement('div');
    control.classList.add('reading');

    if (cmd.type === 'digital') {
      val.setAttribute('data-channel', cmd.command.split('=')[0]);
      const btnOn = document.createElement('button');
      btnOn.className = 'button green';
      btnOn.innerText = 'ON';
      btnOn.onclick = () => sendWSCommand(cmd.command.split('=')[0] + '=ON');

      const btnOff = document.createElement('button');
      btnOff.className = 'button red';
      btnOff.innerText = 'OFF';
      btnOff.onclick = () => sendWSCommand(cmd.command.split('=')[0] + '=OFF');

      control.appendChild(btnOn);
      control.appendChild(btnOff);
    } else if (cmd.type === 'pwm') {
      const slider = document.createElement('input');
      slider.type = 'range';
      slider.min = 0;
      slider.max = 255;
      slider.value = cmd.value;
      slider.oninput = () => {
        val.innerText = `Current: ${slider.value}`;
        sendWSCommand(cmd.command.split('=')[0] + '=' + slider.value);
      };
      control.appendChild(slider);
    }

    div.appendChild(title);
    div.appendChild(val);
    div.appendChild(control);
    container.appendChild(div);
  });
}

function toggleTheme() {
  const body = document.body;
  body.classList.toggle('dark');
  body.classList.toggle('light');
  localStorage.setItem('theme', body.classList.contains('dark') ? 'dark' : 'light');
}

window.onload = () => {
  const savedTheme = localStorage.getItem('theme');
  if (savedTheme) {
    document.body.classList.remove('light', 'dark');
    document.body.classList.add(savedTheme);
  }

  ws = new WebSocket(`ws://${location.hostname}/ws`);
  ws.onopen = () => {
    console.log('WebSocket connected');
    loadChannelInfo();
  };
  ws.onmessage = (msg) => {
    try {
      const data = JSON.parse(msg.data);
      if (data.ack) {
        console.log("ACK:", data.ack);
        const ch = data.ack.split('=')[0];
        const val = data.ack.split('=')[1];
        const el = document.querySelector(`[data-channel="${ch}"]`);
        if (el) {
          if (el.tagName === 'INPUT') {
            el.value = val;
          } else {
            el.innerText = `Current: ${val}`;
          }
        }
      }
    } catch {
      loadChannelInfo(); // fallback
    }
  };
};
