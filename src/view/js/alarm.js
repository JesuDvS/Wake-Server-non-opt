let alarms = [];
let statusCheckInterval = null;

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
    startStatusChecker(); // Cambio: solo verificar estado del servidor
    
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
                    ${alarm.ringing ? ' 🔔' : ''}
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
            showNotification('✅ Alarma creada - Sonará en el servidor', 'success');
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
        loadAlarms(); // Recargar para actualizar estado
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error al detener alarma', 'error');
    }
}

// NUEVO: Verificar estado del servidor (si hay alarma sonando)
function startStatusChecker() {
    statusCheckInterval = setInterval(async () => {
        try {
            const response = await fetch('/api/alarms/status');
            if (!response.ok) return;
            
            const status = await response.json();
            
            if (status.ringing && alarmRingingEl.classList.contains('hidden')) {
                // Mostrar UI de alarma sonando
                ringLabelEl.textContent = status.label || 'Alarma';
                alarmRingingEl.classList.remove('hidden');
                
                // NO reproducir audio aquí - el servidor ya lo está haciendo
                console.log('🔔 Alarma sonando en el servidor:', status.label);
                
            } else if (!status.ringing && !alarmRingingEl.classList.contains('hidden')) {
                // Ocultar UI si se detuvo
                alarmRingingEl.classList.add('hidden');
            }
            
        } catch (error) {
            console.error('Error verificando estado:', error);
        }
    }, 2000); // Verificar cada 2 segundos
}

// Mostrar notificación
function showNotification(message, type) {
    console.log(`[${type.toUpperCase()}] ${message}`);
    
    // Opcional: Agregar toast notification
    const toast = document.createElement('div');
    toast.style.cssText = `
        position: fixed;
        bottom: 20px;
        right: 20px;
        background: ${type === 'success' ? '#28a745' : '#dc3545'};
        color: white;
        padding: 15px 20px;
        border-radius: 8px;
        z-index: 10000;
        animation: slideIn 0.3s ease;
    `;
    toast.textContent = message;
    document.body.appendChild(toast);
    
    setTimeout(() => {
        toast.style.animation = 'slideOut 0.3s ease';
        setTimeout(() => toast.remove(), 300);
    }, 3000);
}

// Prevenir que el navegador se duerma (mantener la UI activa)
function keepAwake() {
    if ('wakeLock' in navigator) {
        navigator.wakeLock.request('screen').catch(err => {
            console.log('Wake Lock no soportado:', err);
        });
    }
}

// Mantener activo al cargar
keepAwake();