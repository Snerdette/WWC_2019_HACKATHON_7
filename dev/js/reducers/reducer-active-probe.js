export default function (state = null, action) {
    switch (action.type) {
        case 'PROBE_SELECTED':
            return action.payload;
            break;
    }
    return state;
}