import {combineReducers} from 'redux';
import ProbeReducer from './reducer-probes';
import ActiveProbeReducer from './reducer-active-probe';


const allReducers = combineReducers({
    probes: ProbeReducer,
    activeUser: ActiveProbeReducer
});

export default allReducers
