import React from 'react';
import ProbeList from '../containers/probe-list';
import ProbeDetails from '../containers/probe-detail';

require('../../scss/style.scss');


const App = () => (
    <div>
        <h2>Probe List</h2>
        <ProbeList />
        <hr />
    </div>
);

export default App;