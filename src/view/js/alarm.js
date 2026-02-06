let alarms = [];
let checkInterval = null;

// Elementos DOM
const currentTimeEl = document.getElementById('currentTime');
const hourInput = document.getElementById('hourInput');
const minuteInput = document.getElementById('minuteInput');
const labelInput = document.getElementById('labelInput');
const vibrateCheck = document.getElementById('vibrateCheck');
const createBtn = document.getElementById('createBtn');
const alarmsListEl = document.getElementById('alarmsList');
const alarmRingingEl = document.getElementById('alarmRinging');
const ringLabelEl = document.getElementById('ringLabel');
const stopAlarmBtn = document.getElementById('stopAlarmBtn');

// Inicialización
document.addEventListener('DOMContentLoaded', () => {
    updateCurrentTime();
    setInterval(updateCurrentTime, 1000);
    loadAlarms();
    startAlarmChecker();
    
    createBtn.addEventListener('click', createAlarm);
    stopAlarmBtn.addEventListener('click', stopAlarm);
    
    // Validación de inputs
    hourInput.addEventListener('input', () => {
        if (hourInput.value > 23) hourInput.value = 23;
        if (hourInput.value < 0) hourInput.value = 0;
    });
    
    minuteInput.addEventListener('input', () => {
        if (minuteInput.value > 59) minuteInput.value = 59;
        if (minuteInput.value < 0) minuteInput.value = 0;
    });
});

// Actualizar reloj
function updateCurrentTime() {
    const now = new Date();
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    currentTimeEl.textContent = `${hours}:${minutes}`;
}

// Cargar alarmas desde el servidor
async function loadAlarms() {
    try {
        const response = await fetch('/api/alarms');
        if (!response.ok) throw new Error('Error al cargar alarmas');
        
        alarms = await response.json();
        renderAlarms();
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error al cargar alarmas', 'error');
    }
}

// Renderizar lista de alarmas
function renderAlarms() {
    if (alarms.length === 0) {
        alarmsListEl.innerHTML = '<p class="empty-state">No hay alarmas configuradas</p>';
        return;
    }
    
    alarmsListEl.innerHTML = '';
    
    alarms.forEach(alarm => {
        const alarmEl = document.createElement('div');
        alarmEl.className = `alarm-item ${!alarm.enabled ? 'disabled' : ''}`;
        
        const hours = String(alarm.hour).padStart(2, '0');
        const minutes = String(alarm.minute).padStart(2, '0');
        
        alarmEl.innerHTML = `
            <div class="alarm-info">
                <div class="alarm-time">${hours}:${minutes}</div>
                <div class="alarm-label">
                    ${alarm.label}${alarm.vibrate ? ' 📳' : ''}
                </div>
            </div>
            <div class="alarm-controls">
                <button class="btn-toggle ${alarm.enabled ? 'active' : ''}" 
                        onclick="toggleAlarm('${alarm.id}')">
                    ${alarm.enabled ? 'ON' : 'OFF'}
                </button>
                <button class="btn-delete" onclick="deleteAlarm('${alarm.id}')">
                    🗑️
                </button>
            </div>
        `;
        
        alarmsListEl.appendChild(alarmEl);
    });
}

// Crear nueva alarma
async function createAlarm() {
    const hour = parseInt(hourInput.value);
    const minute = parseInt(minuteInput.value);
    const label = labelInput.value.trim() || 'Alarma';
    const vibrate = vibrateCheck.checked;
    
    if (isNaN(hour) || isNaN(minute)) {
        showNotification('Por favor ingresa una hora válida', 'error');
        return;
    }
    
    try {
        const response = await fetch('/api/alarms', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ hour, minute, label, vibrate })
        });
        
        if (!response.ok) throw new Error('Error al crear alarma');
        
        const result = await response.json();
        
        if (result.success) {
            showNotification('Alarma creada correctamente', 'success');
            loadAlarms();
            
            // Reset form
            labelInput.value = '';
            vibrateCheck.checked = true;
        }
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error al crear alarma', 'error');
    }
}

// Alternar estado de alarma
async function toggleAlarm(id) {
    try {
        const response = await fetch(`/api/alarms/${id}/toggle`, {
            method: 'PUT'
        });
        
        if (!response.ok) throw new Error('Error al cambiar estado');
        
        const result = await response.json();
        
        if (result.success) {
            loadAlarms();
            showNotification('Estado actualizado', 'success');
        }
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error al actualizar', 'error');
    }
}

// Eliminar alarma
async function deleteAlarm(id) {
    if (!confirm('¿Eliminar esta alarma?')) return;
    
    try {
        const response = await fetch(`/api/alarms/${id}`, {
            method: 'DELETE'
        });
        
        if (!response.ok) throw new Error('Error al eliminar');
        
        const result = await response.json();
        
        if (result.success) {
            loadAlarms();
            showNotification('Alarma eliminada', 'success');
        }
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error al eliminar', 'error');
    }
}

// Detener alarma sonando
async function stopAlarm() {
    try {
        const response = await fetch('/api/alarms/stop', {
            method: 'POST'
        });
        
        if (!response.ok) throw new Error('Error al detener alarma');
        
        alarmRingingEl.classList.add('hidden');
        showNotification('Alarma detenida', 'success');
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error al detener alarma', 'error');
    }
}

// Verificar alarmas activas (frontend check)
function startAlarmChecker() {
    checkInterval = setInterval(() => {
        const now = new Date();
        const currentHour = now.getHours();
        const currentMinute = now.getMinutes();
        
        alarms.forEach(alarm => {
            if (alarm.enabled && 
                alarm.hour === currentHour && 
                alarm.minute === currentMinute) {
                
                // Mostrar pantalla de alarma
                ringLabelEl.textContent = alarm.label;
                alarmRingingEl.classList.remove('hidden');
                
                // Vibración web si está soportada
                if ('vibrate' in navigator && alarm.vibrate) {
                    navigator.vibrate([500, 200, 500, 200, 500]);
                }
                
                // Reproducir sonido
                playAlarmSound();
            }
        });
    }, 5000); // Check cada 5 segundos
}

// Reproducir sonido de alarma
function playAlarmSound() {
    // Crear contexto de audio
    try {
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
        
        // Generar tono de alarma
        const oscillator = audioContext.createOscillator();
        const gainNode = audioContext.createGain();
        
        oscillator.connect(gainNode);
        gainNode.connect(audioContext.destination);
        
        oscillator.frequency.value = 800; // Hz
        oscillator.type = 'sine';
        
        gainNode.gain.setValueAtTime(0.3, audioContext.currentTime);
        
        oscillator.start(audioContext.currentTime);
        oscillator.stop(audioContext.currentTime + 0.5);
        
        // Repetir cada segundo
        const intervalId = setInterval(() => {
            if (!alarmRingingEl.classList.contains('hidden')) {
                const osc = audioContext.createOscillator();
                const gain = audioContext.createGain();
                
                osc.connect(gain);
                gain.connect(audioContext.destination);
                
                osc.frequency.value = 800;
                osc.type = 'sine';
                gain.gain.setValueAtTime(0.3, audioContext.currentTime);
                
                osc.start(audioContext.currentTime);
                osc.stop(audioContext.currentTime + 0.5);
            } else {
                clearInterval(intervalId);
            }
        }, 1000);
        
    } catch (error) {
        console.error('Error reproduciendo audio:', error);
    }
}

// Mostrar notificación
function showNotification(message, type) {
    // Simple console log - puedes implementar notificaciones visuales
    console.log(`[${type.toUpperCase()}] ${message}`);
}

// Prevenir que el navegador se duerma
function keepAwake() {
    if ('wakeLock' in navigator) {
        navigator.wakeLock.request('screen').catch(err => {
            console.log('Wake Lock no soportado:', err);
        });
    }
}

// Mantener activo al cargar
keepAwake();