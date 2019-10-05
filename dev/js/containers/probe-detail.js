import React, {Component} from 'react';
import {connect} from 'react-redux';


class ProbeDetail extends Component {

    renderDetails() {
        return this.state.probe.map((probe) => {
            return (
                <li key={probe.id} onClick={() => this.props.selectProbe(probe)}>
                    {probe.id} : {probe.location}
                </li>
            );
        });
    }

    render() {
        if (!this.props.probe) {
            return (<div>Select a Probe</div>);
        }
        return (
            <div>
                <h2>ID: {this.props.probe.id}</h2>
                <h3>Location: {this.props.probe.location}</h3>
                <h3>Status: {this.props.probe.status}</h3>
                <h3>Install Date: {this.props.probe.installDate}</h3>
            </div>
        );
    }
}

// "state.activeProbe" is set in reducers/index.html
function mapStateToProps(state) {
    return {
        probe: state.activeProbe
    };
}

export default connect(mapStateToProps)(ProbeDetail);
