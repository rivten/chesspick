import { h, render, Component } from './preact.mjs';
import htm from './htm.mjs';

const html = htm.bind(h);

class Clock extends Component {
    state = { time: Date.now() }

    componentDidMount() {
        this.timer = setInterval(
            () => {
                this.setState({ time: Date.now() });
            }, 1000
        );
    }

    componentWillUnmount() {
        clearInterval(this.timer);
    }

    render() {
        let time = new Date().toLocaleTimeString();
        return html`
            <span>${time}</span>
        `;
    }
}

class App extends Component {
    state = { value: '', name: 'sailor' }

    onInput = ev => {
        this.setState({ value: ev.target.value });
    }

    onSubmit = ev => {
        ev.preventDefault()

        this.setState({ name: this.state.value });
    }

    render() {
        return html`
            <${Clock} />
            <div>
                <h1>Hello ${this.state.name} !</h1>
                <form onSubmit=${this.onSubmit}>
                    <input type="text" value=${this.state.value} onInput=${this.onInput} />
                    <button type="submit">Update</button>
                </form>
            </div>
        `;
    }
}

render(html`<${App} />`, document.body);
