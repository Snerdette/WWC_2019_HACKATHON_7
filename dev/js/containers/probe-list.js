import React, {Component} from 'react';
import {bindActionCreators} from 'redux';
import {connect} from 'react-redux';
import {selectProbe} from "../actions/index";

class ProbeList extends Component {
    renderList() {
        return this.props.probes.map((probe) => {
            return (
                <li key={probe.id} onClick={() => this.props.selectProbe(probe)}>
                    {probe.id} : {probe.location}
                </li>
            );
        });
    }

    render () {
        return (
            <ul>
                {this.renderList()}
            </ul>
        );
    }
}

function mapStateToProps(state) {
    return {
        probes: state.probes
    };
}

// Get actions and pass them as props to to UserList
//      > now UserList has this.props.selectUser
function matchDispatchToProps(dispatch){
    return bindActionCreators({selectProbe: selectProbe}, dispatch);
}

// We don't want to return the plain UserList (component) anymore, we want to return the smart Container
//      > UserList is now aware of state and actions
export default connect(mapStateToProps, matchDispatchToProps)(ProbeList);
